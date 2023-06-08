#pragma once

template <typename T>
class Position
{
public:
    Position()
      : x_{}
      , y_{}
    {
    }

    Position(T x, T y)
      : x_(x)
      , y_(y)
    {
    }

    void setX(T x)
    {
        x_ = x;
    }

    void setY(T y)
    {
        y_ = y;
    }

    [[nodiscard]] T getX() const
    {
        return x_;
    }

    [[nodiscard]] T getY() const
    {
        return y_;
    }

private:
    T x_, y_;
};
