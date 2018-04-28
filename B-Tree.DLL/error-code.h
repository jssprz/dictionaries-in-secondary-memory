#pragma once

namespace btree {
	
	enum Error_code {
		success=0,
		not_present=1,
		overflow=2,
		duplicate_error=3
	};
}