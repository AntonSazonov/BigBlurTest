#pragma once

#include "cpu_features/include/cpuinfo_x86.h"

namespace san::cpu {

class features {
	const cpu_features::X86Info					m_info;
	//const cpu_features::X86Microarchitecture	m_arch;

public:
	features()
		: m_info( cpu_features::GetX86Info() )
		//, m_arch( cpu_features::GetX86Microarchitecture( &g_info ) )
	{}

	std::string CPU() const {
		std::string name( m_info.brand_string );
		name.erase( name.find_last_not_of( ' ' ) + 1 ); // Remove trailing spaces
		return name;
	}

	std::string SIMD() const {
		std::string simds;
		if ( SSE2()  ) simds += " SSE2";
		if ( SSE3()  ) simds += " SSE3";
		if ( SSSE3() ) simds += " SSSE3";
		if ( SSE41() ) simds += " SSE4.1";
		if ( SSE42() ) simds += " SSE4.2";
		if ( AVX()   ) simds += " AVX";
		if ( AVX2()  ) simds += " AVX2";
		return simds;
	}

	bool SSE2()		const { return m_info.features.sse2; }
	bool SSE3()		const { return m_info.features.sse3; }
	bool SSSE3()	const { return m_info.features.ssse3; }
	bool SSE41()	const { return m_info.features.sse4_1; }
	bool SSE42()	const { return m_info.features.sse4_2; }
	bool AVX()		const { return m_info.features.avx; }
	bool AVX2()		const { return m_info.features.avx2; }
}; // class features

} // namespace san::cpu
