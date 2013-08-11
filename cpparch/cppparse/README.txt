

The standard library provided with Visual Studio 2005 has a few bugs in it which the MSVC compiler doesn't find.
Many of these bugs appear to have been introduced by security fixes.
In order to parse it with a compliant parser, we need to fix them.


<stdio.h>
	__DEFINE_CPP_OVERLOAD_SECURE_FUNC_0_2_ARGLIST: template-arg '_Size' conflicts with param name (multiple instances)
	_vsnprintf_s used before definition (multiple instances) 307, 437, 445

<iterator> and <xutility>
	std::out_of_range not included before use

<iterator>
	stdext::make_checked_array_iterator is used before declaration

<vector>
	add 'typename' for usage of _MycontTy::size_type _Vb_iter_base::_My_actual_offset()
	2172: _Vbtype::iterator missing 'typename'

<xutility>
	before SP1: std::checked_fill_n does not compile!
	 http://connect.microsoft.com/VisualStudio/feedback/details/99456/checked-fill-n-error-c2065-dest-undeclared-identifier
	after SP1: missing _STD in call to _Fill_n in checked_fill_n (2 instances)
	char_traits::_Copy_s uses ::_invalid_parameter before declaration

<memory>
	stdext::unchecked_uninitialized_copy used before declaration
		replace with _Uninit_copy

<algorithm>
	stdext::unchecked_remove_copy used before declaration
	stdext::checked_swap_ranges fails to compile
	std::_Merge_backward fails to compile

