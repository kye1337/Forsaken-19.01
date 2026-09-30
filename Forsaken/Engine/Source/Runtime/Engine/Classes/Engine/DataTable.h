#pragma once

#include "CoreGlobals.h"

class DataTable
{
public:
	template<typename RowType>
	static inline float GetTotalWeight(const TArray<RowType*>& Elements)
	{
		float TotalWeight = 0.f;

		for (RowType* Element : Elements)
		{
			if (Element == NULL)
				continue;

			TotalWeight += Element->Weight;
		}

		return TotalWeight;
	}

	template<typename RowType>
	static inline RowType* PickWeightedElement(const TArray<RowType*>& Elements, float RandomWeight)
	{
		for (RowType* Element : Elements)
		{
			if (Element == NULL)
				continue;

			float Weight = Element->Weight;

			if (Weight == 0)
				continue;

			if (RandomWeight <= Weight)
				return Element;

			RandomWeight -= Weight;
		}

		return NULL;
	}
};
