// Copyright June Rhodes. All Rights Reserved.

#pragma once

#include "RedpointEOSBuild/BuildEnvironment.h"
#if !REDPOINT_EOS_IS_FREE_EDITION
#include "Templates/Less.h"
#endif

REDPOINT_EOS_CODE_GUARD_BEGIN()

#if !REDPOINT_EOS_IS_FREE_EDITION

namespace Openskill
{
template <typename T, typename U> struct FAnchoredElement
{
public:
    int32 OriginalIndex;
    U Rank;
    const T &Element;
    FAnchoredElement(const int32 &InOriginalIndex, const U &InRank, const T &InElement)
        : OriginalIndex(InOriginalIndex)
        , Rank(InRank)
        , Element(InElement)
    {
    }
    friend bool operator<(const FAnchoredElement<T, U> &A, const FAnchoredElement<T, U> &B)
    {
        return A.Rank < B.Rank;
    }
};

template <typename T, typename U>
void Unwind(
    const TArray<U> &Ranks,
    const TArray<T> &Elements,
    TArray<int32> &OriginalIndexes,
    TArray<T> &SortedElements,
    TArray<U> &SortedRanks)
{
    OriginalIndexes.Empty();
    SortedElements.Empty();

    checkf(Ranks.Num() == Elements.Num(), TEXT("Ranks count must be equal to elements count!"));

    // Generate a combined array that stores the original index, intended rank and element value.
    TArray<FAnchoredElement<T, U>> AnchoredElements;
    for (int32 i = 0; i < Ranks.Num(); i++)
    {
        AnchoredElements.Add(FAnchoredElement<T, U>(i, Ranks[i], Elements[i]));
    }

    // Sort the anchored elements by their rank increasing (rank 0 is at the start).
    Algo::Sort(AnchoredElements, TLess<FAnchoredElement<T, U>>());

    // Emit the elements in the order that they were found.
    for (int32 i = 0; i < Ranks.Num(); i++)
    {
        OriginalIndexes.Add(AnchoredElements[i].OriginalIndex);
        SortedElements.Add(AnchoredElements[i].Element);
        SortedRanks.Add(AnchoredElements[i].Rank);
    }
}
}; // namespace Openskill

#endif

REDPOINT_EOS_CODE_GUARD_END()