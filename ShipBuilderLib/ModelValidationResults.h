/***************************************************************************************
* Original Author:		Gabriele Giuseppini
* Created:				2021-11-05
* Copyright:			Gabriele Giuseppini  (https://github.com/GabrieleGiuseppini)
***************************************************************************************/
#pragma once

#include <algorithm>
#include <vector>

namespace ShipBuilder {

class ModelValidationIssue final
{
public:

    enum class Type
    {
        MissingElectricalSubstrate
    };

    ModelValidationIssue(
        Type type,
        bool isError)
        : mType(type)
        , mIsError(isError)
    {}

    Type GetType() const
    {
        return mType;
    }

    bool IsError() const
    {
        return mIsError;
    }

private:

    Type mType;
    bool mIsError;
};

class ModelValidationResults final
{
public:

    ModelValidationResults()
        : mIssues()
    {
    }

    ModelValidationResults(std::vector<ModelValidationIssue> && issues)
        : mIssues(std::move(issues))
    {
    }

    bool HasErrors() const
    {
        return std::find_if(
            mIssues.cbegin(),
            mIssues.cend(),
            [](ModelValidationIssue const & issue)
            {
                return issue.IsError();
            }) != mIssues.cend();
    }

    bool IsEmpty() const
    {
        return mIssues.empty();
    }

    auto const & GetIssues() const
    {
        return mIssues;
    }

private:

    std::vector<ModelValidationIssue> mIssues;
};
}