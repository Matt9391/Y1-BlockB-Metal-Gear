#include "common/adapter_scoring.h"

namespace buz::rhi
{
	//--------------------------------------------------------------
	const char* describe(adapter_preference preference)
	{
		switch (preference)
		{
		case adapter_preference::highest_score: return "highest scoring";
		case adapter_preference::lowest_score: return "lowest scoring";
		case adapter_preference::software: return "software rasterizer";
		}
		return "unknown";
	}
}
