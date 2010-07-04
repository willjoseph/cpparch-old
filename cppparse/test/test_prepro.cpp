
namespace N
{
	struct S;

	typedef S ios;

	struct S
	{
		static const int skipws = 0;
	};
}

int i = N::ios::skipws;


#if 0
namespace N32
{
	namespace std
	{
		template<class _Dummy>
		class _Iosb
		{
		public:
			enum _Fmtflags
			{
				_Fmtmask=0xffff, _Fmtzero=0
			};
			static const _Fmtflags skipws=(_Fmtflags)0x0001;
		};
		class ios_base: public _Iosb<int>
		{
		};
		template<class _Elem, class _Traits>
		class basic_ios: public ios_base
		{
		};
		struct _Char_traits_base
		{
		};
		template<typename T>
		struct char_traits : public _Char_traits_base
		{
		};
		template<>
		struct char_traits<char> : public _Char_traits_base
		{
		};
		typedef basic_ios<char, char_traits<char> >ios;
	}

	namespace wave
	{
		enum language_support
		{
			support_normal=0x01, support_cpp=support_normal, support_option_long_long=0x02, support_option_variadics=0x04, support_c99=support_option_variadics|support_option_long_long|0x08, support_option_mask=0xFF80, support_option_insert_whitespace=0x0080, support_option_preserve_comments=0x0100, support_option_no_character_validation=0x0200, support_option_convert_trigraphs=0x0400, support_option_single_line=0x0800, support_option_prefer_pp_numbers=0x1000, support_option_emit_line_directives=0x2000, support_option_include_guard_detection=0x4000, support_option_emit_pragma_directives=0x8000
		};
		namespace iteration_context_policies
		{
			struct load_file_to_string
			{
				template<typename IterContextT>
				class inner
				{
				public:
					template<typename PositionT>
					static void init_iterators(IterContextT&iter_ctx, PositionT const&act_pos, language_support language)
					{
						int i = std::ios::skipws;
					}
				};
			};

			void f()
			{
				int i;
				load_file_to_string::inner<int>::init_iterators(i, i, support_normal);
			}
		}
	}
}

#endif
