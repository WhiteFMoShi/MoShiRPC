#pragma once

template <class T>
class Result {
    void SetRusult(const T&);
    const T& Get();
private:
    int status;
    T value_;
};