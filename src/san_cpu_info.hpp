#pragma once

namespace san {

// Basic CPU info, such as Vendor, Brand and SIMD support.
class cpu_info {
	using i32x4 = int32_t [4];

	int				m_funcs_num;
	i32x4			m_funcs[8];			// I don't need higher functions...

	int				m_funcs_ext_num;
	i32x4			m_funcs_ext[5];		// ...

	std::string		m_vendor;
	std::string		m_brand;
	std::string		m_feats;

	enum class feat_e : uint8_t { /*SSE,*/ SSE2, SSE3, SSSE3, SSE41/*, SSE42*/, FMA3, AVX, AVX2/*, BMI1, BMI2*/, FEAT_E_MAX };
	enum class  reg_e : uint8_t { eax, ebx, ecx, edx };

#define DEF_FEATURE( feat, fun, reg, bit ) { feat_e::feat, fun, reg, bit, #feat }

	struct {
		feat_e		feat;
		uint8_t		fun : 3;
		reg_e		reg : 2;
		uint8_t		bit : 5;
		std::string	name;
	} m_features[static_cast<size_t>( feat_e::FEAT_E_MAX )] = {

		//DEF_FEATURE(   SSE, 1, reg_e::edx, 25 ),
		DEF_FEATURE(  SSE2, 1, reg_e::edx, 26 ),

		DEF_FEATURE(  SSE3, 1, reg_e::ecx,  0 ),
		DEF_FEATURE( SSSE3, 1, reg_e::ecx,  9 ),
		DEF_FEATURE( SSE41, 1, reg_e::ecx, 19 ),
		//DEF_FEATURE( SSE42, 1, reg_e::ecx, 20 ),
		DEF_FEATURE(  FMA3, 1, reg_e::ecx, 12 ),
		DEF_FEATURE(   AVX, 1, reg_e::ecx, 28 ),

		DEF_FEATURE(  AVX2, 7, reg_e::ebx,  5 ),
		//DEF_FEATURE(  BMI1, 7, reg_e::ebx,  3 ),
		//DEF_FEATURE(  BMI2, 7, reg_e::ebx,  8 )
	};

#undef DEF_FEATURE

public:
	cpu_info() {

		// EAX=0: Highest Function Parameter and Manufacturer ID
		i32x4 data;
		__cpuid( data, 0 );
		m_funcs_num = data[0] + 1;
		if ( m_funcs_num > 8 ) m_funcs_num = 8;	// Don't need more...
		for ( int i = 0; i < m_funcs_num; ++i ) {
			__cpuidex( m_funcs[i], i, 0 );
		}

		// Get vendor string
		std::swap( m_funcs[0][2], m_funcs[0][3] ); // Swap EDX and ECX
		m_vendor = std::string( reinterpret_cast<const char *>( &m_funcs[0][1] ), 12 );

		// EAX=80000000h: Get Highest Extended Function Implemented
		__cpuid( data, 0x80000000 );
		m_funcs_ext_num = data[0] - 0x80000000 + 1;
		if ( m_funcs_ext_num > 5 ) m_funcs_ext_num = 5;	// Don't need more...
		for ( int i = 0; i < m_funcs_ext_num; ++i ) {
			__cpuidex( m_funcs_ext[i], i + 0x80000000, 0 );
		}

		// Get brand string
		if ( m_funcs_ext_num - 1 >= 4 ) {
			const char * p_brand = reinterpret_cast<const char *>( &m_funcs_ext[2][0] );
			const char * p = p_brand + 47;	// Whole string is 48 bytes long. 47th byte is '\0'.
			while ( *--p == ' ' );			// Trim trailing spaces.
			m_brand = std::string( p_brand, p - p_brand + 1 );
		}

		// Get features string
		for ( const auto & feat : m_features ) {
			if ( m_funcs[feat.fun][static_cast<size_t>( feat.reg )] & 1 << feat.bit ) {
				m_feats += m_feats.empty() ? feat.name : ' ' + feat.name;
			}
		}
	}

	std::string	vendor() const { return m_vendor; }
	std::string	brand()  const { return m_brand;  }
	std::string	feats()  const { return m_feats;  }

	//bool sse()		const { return m_funcs[1][static_cast<size_t>( reg_e::edx )] & 1 << 25; }
	bool sse2()		const { return m_funcs[1][static_cast<size_t>( reg_e::edx )] & 1 << 26; }

	bool sse3()		const { return m_funcs[1][static_cast<size_t>( reg_e::ecx )] & 1 <<  0; }
	bool ssse3()	const { return m_funcs[1][static_cast<size_t>( reg_e::ecx )] & 1 <<  9; }
	bool sse41()	const { return m_funcs[1][static_cast<size_t>( reg_e::ecx )] & 1 << 19; }
	//bool sse42()	const { return m_funcs[1][static_cast<size_t>( reg_e::ecx )] & 1 << 20; }
	bool fma3()		const { return m_funcs[1][static_cast<size_t>( reg_e::ecx )] & 1 << 12; }
	bool avx()		const { return m_funcs[1][static_cast<size_t>( reg_e::ecx )] & 1 << 28; }

	bool avx2()		const { return m_funcs[7][static_cast<size_t>( reg_e::ebx )] & 1 <<  5; }
	//bool bmi1()		const { return m_funcs[7][static_cast<size_t>( reg_e::ebx )] & 1 <<  3; }
	//bool bmi2()		const { return m_funcs[7][static_cast<size_t>( reg_e::ebx )] & 1 <<  8; }

	//bool sse4a()	const { return m_funcs_ext[1][static_cast<size_t>( reg_e::ecx )] & 1 <<  6; }
	//bool fma4()		const { return m_funcs_ext[1][static_cast<size_t>( reg_e::ecx )] & 1 << 16; }
}; // class cpu_info

} // namespace san
