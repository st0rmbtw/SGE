#include <algorithm>

#include <LLGL/PipelineLayout.h>
#include <LLGL/PipelineLayoutFlags.h>
#include <LLGL/PipelineState.h>
#include <LLGL/PipelineStateFlags.h>

#include <SGE/assert.hpp>
#include <SGE/profile.hpp>
#include <SGE/renderer/attributes.hpp>
#include <SGE/renderer/batch.hpp>
#include <SGE/renderer/buffer_pool.hpp>
#include <SGE/renderer/renderer2d.hpp>
#include <SGE/renderer/types.hpp>
#include <SGE/renderer/utils.hpp>
#include <SGE/types/binding_layout.hpp>
#include <SGE/utils/hash.hpp>

namespace {

using sge::internal::SortItem;

// Stable LSD radix sort on the 64-bit key. Passes where all keys share the byte are skipped, so
// typically only 3-4 of the 8 passes run (order's low bytes + state id).
void RadixSortStable(std::vector<SortItem>& a, std::vector<SortItem>& tmp) {
    const size_t n = a.size();

    // For a small array use insertion sort
    if (n <= 64) {
        size_t min_idx = 0;
        for (size_t i = 1; i < n; i++) {
            if (a[i].key < a[min_idx].key) {
                min_idx = i;
            }
        }
        auto min_val = a[min_idx].key;
        a[min_idx] = a[0];
        a[0].key = min_val;

        for (size_t i = 2; i < n; i++) {
            auto k = a[i].key;
            auto j = i;
            while (k < a[j - 1].key) {
                a[j] = a[j - 1];
                j--;
            }
            a[j].key = k;
        }
        return;
    }

    tmp.resize(n);

    std::array<std::array<uint32_t, 256>, 8> hist{};
    for (const SortItem& it : a)
        for (int p = 0; p < 8; ++p) ++hist[p][(it.key >> (p * 8)) & 0xFF];

    SortItem* src = a.data();
    SortItem* dst = tmp.data();
    for (int p = 0; p < 8; ++p) {
        if (hist[p][(src[0].key >> (p * 8)) & 0xFF] == n) continue;       // single bucket: nothing to do
        std::array<uint32_t, 256> offs;
        uint32_t sum = 0;
        for (int i = 0; i < 256; ++i) {
            offs[i] = sum;
            sum += hist[p][i];
        }
        for (size_t i = 0; i < n; ++i)
            dst[offs[(src[i].key >> (p * 8)) & 0xFF]++] = src[i];
        std::swap(src, dst);
    }
    if (src != a.data())
        std::memcpy(a.data(), src, n * sizeof(SortItem));
}

} // namespace

sge::Renderer2D::Renderer2D(const std::shared_ptr<RenderContext>& context) : Renderer(context) {
}

void sge::Renderer2D::SubmitBatch(std::shared_ptr<IBatch> batch) {
    ZoneScoped;

    auto runs = batch->GetRuns();
    if (runs.empty()) return;

    const auto& material = batch->GetMaterial(sge::BlendMode::AlphaBlend);
    const size_t instanceStride = material->GetInstanceAttribs()[0].stride;
    const auto* instancesData = static_cast<const uint8_t*>(batch->GetInstanceData());

    internal::BatchSubmission submission;
    submission.states = &batch->GetStates();
    submission.batch = std::move(batch);
    submission.instanceStride = instanceStride;

    auto sortPredicate = [](const internal::BatchRun& a, const internal::BatchRun& b) {
        return a.key < b.key;
    };

    if (std::ranges::is_sorted(runs, sortPredicate)) {
        // Painter-ordered input is already sorted: zero sorting, zero copying.
        submission.runsSpan = runs;
        submission.instancesBytes = instancesData;
    } else {
        const size_t n = runs.size();
        m_sort_items.resize(n);
        for (size_t i = 0; i < n; ++i)
            m_sort_items[i] = { .key=runs[i].key, .index=static_cast<uint32_t>(i) };

        {
            ZoneScopedN("BatchSort");
            RadixSortStable(m_sort_items, m_sort_tmp);
        }

        if (m_scratch_used >= m_scratch_pool.size())
            m_scratch_pool.resize(m_scratch_used + 1);

        internal::SortScratch& scratch = m_scratch_pool[m_scratch_used++];
        scratch.runs.resize(n);
        scratch.instances.resize(n * instanceStride);

        // Gather: sequential writes
        uint8_t* dst = scratch.instances.data();
        for (size_t k = 0; k < n; ++k) {
            const auto& run = runs[m_sort_items[k].index];
            scratch.runs[k].key = run.key;
            scratch.runs[k].first = run.first;
            scratch.runs[k].count = run.count;
            std::memcpy(dst, instancesData + run.first * instanceStride, run.count * instanceStride);
            dst += run.count * instanceStride;
        }
        submission.runsSpan = scratch.runs;
        submission.instancesBytes = scratch.instances.data();
    }

    m_submissions.push_back(std::move(submission));
}

void sge::Renderer2D::FlushBatches() {
    ZoneScoped;

    while (true) {
        // Get the submission whose next draw has the lowest order
        internal::BatchSubmission* cur = nullptr;
        uint32_t curOrder = 0;
        for (auto& s : m_submissions) {
            if (s.cursor >= s.runsSpan.size()) continue;
            const uint32_t o = internal::SortKeyOrder(s.runsSpan[s.cursor].key);
            if (!cur || o < curOrder) {
                cur = &s;
                curOrder = o;
            }
        }
        if (!cur) break;

        const uint32_t stateId = internal::SortKeyState(cur->runsSpan[cur->cursor].key);
        const auto& st = cur->states->Get(stateId);

        for (const internal::BatchRun& run : cur->runsSpan) {
            SGE_ASSERT(run.first < run.count);

            SubmitManyRaw(
                cur->batch->GetMesh(),
                cur->batch->GetMaterial(st.blendMode),
                st.Resources(),
                st.scissor,
                cur->instancesBytes + run.first * cur->instanceStride,
                cur->instanceStride,
                run.count
            );
            cur->cursor += 1;
        }
    }

    m_submissions.clear();
}