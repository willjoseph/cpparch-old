
template<typename T>
typename T::dependent f(typename T::dependent t)
{
	dependent1(T::dependent2());
}

