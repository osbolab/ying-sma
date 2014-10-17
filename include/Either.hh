#pragma once

namespace sma
{

template <class T1, class T2>
class Either
{
  bool isLeft;
  union
  {
    T1 leftVal;
    T2 rightVal;
  };
  template<class T1_, class T2_> friend Either<T1_, T2_> Left(T1_ x);
  template<class T1_, class T2_> friend Either<T1_, T2_> Right(T1_ x);
public:
  bool left(T1& x)
  {
    if (isLeft)
      x = leftVal;
    return isLeft;
  }
  bool right(T2& x)
  {
    if (!isLeft)
      x = rightVal;
    return !isLeft;
  }
};

template <class T1, class T2>
Either<T1, T2> Left(T1 x)
{
  Either<T1, T2> e;
  e.isLeft = true;
  e.leftVal = x;
  return e;
}
template <class T1, class T2>
Either<T1, T2> Right(T2 x)
{
  Either<T1, T2> e;
  e.isLeft = false;
  e.rightVal = x;
  return e;
}

}