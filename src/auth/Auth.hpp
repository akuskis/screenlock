#pragma once

class Auth
{
  public:
    virtual ~Auth() = default;

    [[nodiscard]] virtual bool is_correct_password(char const* password) const = 0;
};
