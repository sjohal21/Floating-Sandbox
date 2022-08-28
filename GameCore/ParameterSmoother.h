/***************************************************************************************
 * Original Author:		Gabriele Giuseppini
 * Created:				2019-09-29
 * Copyright:			Gabriele Giuseppini  (https://github.com/GabrieleGiuseppini)
 ***************************************************************************************/
#pragma once

#include <cassert>
#include <functional>

template<typename TValue>
class ParameterSmoother
{
public:

    ParameterSmoother(
        std::function<TValue const &()> getter,
        std::function<void(TValue const &)> setter,
        float convergenceFactor)
        : ParameterSmoother(
            getter,
            [setter](TValue const & value) -> TValue const &
            {
                setter(value);
                return value;
            },
            [](TValue const & value) -> TValue
            {
                return value;
            },
            convergenceFactor)
    {}

    ParameterSmoother(
        std::function<TValue const &()> getter,
        std::function<TValue const &(TValue const &)> setter,
        std::function<TValue(TValue const &)> clamper,
        float convergenceFactor)
        : mGetter(std::move(getter))
        , mSetter(std::move(setter))
        , mClamper(std::move(clamper))
        , mConvergenceFactor(convergenceFactor)
    {
        mCurrentValue = mTargetValue = mGetter();
    }

    /*
     * Returns the current value, which is the target value as smoothing is assumed to happen "offline".
     */
    TValue const & GetValue() const
    {
        return mTargetValue;
    }

    void SetValue(TValue const & value)
    {
        assert(mCurrentValue == mGetter());
        mTargetValue = mClamper(value);
    }

    void SetValueImmediate(TValue const & value)
    {
        mCurrentValue = mTargetValue = mSetter(value);
    }

    void ReClamp()
    {
        mTargetValue = mClamper(mTargetValue);
    }

    void Update()
    {
        if (mCurrentValue != mTargetValue)
        {
            // Converge
            mCurrentValue += (mTargetValue - mCurrentValue) * mConvergenceFactor;

            // See if close enough
            if (Distance(mCurrentValue, mTargetValue) < 0.0001f)
            {
                // Reached target
                mCurrentValue = mTargetValue;
            }

            // Set
            mCurrentValue = mSetter(mCurrentValue);

            // In case conditions have changed, we pickup the new target value
            // and we will return the correct value
            mTargetValue = mClamper(mTargetValue);
        }
    }

    void SetConvergenceFactor(float value)
    {
        mConvergenceFactor = value;
    }

private:

    float Distance(float value1, float value2) const
    {
        return std::abs(value1 - value2);
    }

    float Distance(vec2f const & value1, vec2f const & value2) const
    {
        return (value1 - value2).length();
    }

private:

    std::function<TValue const & ()> const mGetter;
    std::function<TValue const &(TValue const &)> const mSetter;
    std::function<TValue(TValue const &)> const mClamper;
    float mConvergenceFactor;

    TValue mCurrentValue;
    TValue mTargetValue; // This is also the new official storage of the parameter value
};
