#pragma once

namespace common {
	
	enum Error_code {
		success=0,
		not_present=1,
		overflow=2,
		duplicate_error=3,
		not_implemented=4
	};

	inline const char* ToString(Error_code v)
	{
		switch (v)
		{
		case success:   return "success";
		case not_present:   return "not_present";
		case overflow: return "overflow";
		case duplicate_error: return "duplicate_error";
		default:      return "[Unknown error code]";
		}
	}
}