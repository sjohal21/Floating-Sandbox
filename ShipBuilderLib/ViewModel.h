/***************************************************************************************
* Original Author:		Gabriele Giuseppini
* Created:				2021-08-28
* Copyright:			Gabriele Giuseppini  (https://github.com/GabrieleGiuseppini)
***************************************************************************************/
#pragma once

#include "ShipBuilderTypes.h"

#include <algorithm>
#include <cmath>

namespace ShipBuilder {

/*
 * This class maintains the logic for transformations between the various coordinates.
 *
 * Terminology:
 *  - WorkSpace: has the pixel size of the structure (equivalent of ::World)
 *  - WorkSpaceCoordinates
 *  - DisplayLogical: has the logical display (window) size (equivalent of ::Logical)
 *  - DisplayLogicalCoordinates: the logical display coordinates (type @ ShipBuilderTypes)
 *  - DisplayPixel: has the pixel display (window) size (equivalent of ::Pixel)
 *  - DisplayPixelCoordinates: the pixel display coordinates (type @ ShipBuilderTypes)
 */
class ViewModel
{
public:

    using ProjectionMatrix = float[4][4];

public:

    ViewModel(
        DisplayLogicalSize initialDisplaySize,
        int logicalToPhysicalPixelFactor)
        : mZoom(0)
        , mCam(0, 0)
        , mLogicalToPhysicalPixelFactor(logicalToPhysicalPixelFactor)
        , mDisplayLogicalSize(initialDisplaySize)
        , mDisplayPhysicalSize(
            initialDisplaySize.width * logicalToPhysicalPixelFactor,
            initialDisplaySize.height * logicalToPhysicalPixelFactor)
    {
        //
        // Initialize ortho matrix
        //

        std::fill(
            &(mOrthoMatrix[0][0]),
            &(mOrthoMatrix[0][0]) + sizeof(mOrthoMatrix) / sizeof(float),
            0.0f);

        mOrthoMatrix[3][3] = 1.0f;

        //
        // Calculate all attributes based on current view
        //

        RecalculateAttributes();
    }

    int GetZoom() const
    {
        return mZoom;
    }

    int SetZoom(int zoom)
    {
        mZoom = Clamp(zoom, MinZoom, MaxZoom);

        RecalculateAttributes();

        return mZoom;
    }

    WorkSpaceCoordinates const & GetCameraWorkSpacePosition() const
    {
        return mCam;
    }

    WorkSpaceCoordinates const & SetCameraWorkSpacePosition(WorkSpaceCoordinates const & pos)
    {
        mCam = pos;

        RecalculateAttributes();

        return mCam;
    }

    DisplayPhysicalSize const & GetDisplayPhysicalSize() const
    {
        return mDisplayPhysicalSize;
    }

    void SetDisplayLogicalSize(DisplayLogicalSize const & logicalSize)
    {
        mDisplayLogicalSize = logicalSize;

        mDisplayPhysicalSize = DisplayPhysicalSize(
            logicalSize.width * mLogicalToPhysicalPixelFactor,
            logicalSize.height * mLogicalToPhysicalPixelFactor);

        RecalculateAttributes();
    }

    WorkSpaceSize GetCameraPanRange() const
    {
        // Return work space that fits entirely in current display, considering margins

        return WorkSpaceSize(
            DisplayLogicalToWorkSpace(mDisplayLogicalSize.width) - static_cast<int>(2.0f * mMarginWorkSize),
            DisplayLogicalToWorkSpace(mDisplayLogicalSize.height) - static_cast<int>(2.0f * mMarginWorkSize));
    }

    //
    // Coordinate transformations
    //

    WorkSpaceCoordinates DisplayLogicalToWorkSpace(DisplayLogicalCoordinates const & displayCoordinates) const
    {
        return WorkSpaceCoordinates(
            DisplayLogicalToWorkSpace(displayCoordinates.x),
            DisplayLogicalToWorkSpace(displayCoordinates.y));
    }

    ProjectionMatrix const & GetOrthoMatrix() const
    {
        return mOrthoMatrix;
    }

private:

    void RecalculateAttributes()
    {
        // Zoom factor
        mZoomFactor = CalculateZoomFactor(mZoom);

        // Margin work size
        mMarginWorkSize = 8.0f * mZoomFactor;

        // Ortho Matrix:
        //  WorkCoordinates * OrthoMatrix => NDC
        //
        //  Work: (0, W/H) (positive right-bottom)
        //  NDC : (-1.0, +1.0) (positive right-top)
        //
        // We add a (left, top) margin whose physical pixel size equals the physical pixel
        // size if one work space pixel at max zoom
        //
        // SDsp is display scaled by zoom
        //
        //  2 / SDspW                0                        0                0
        //  0                        -2 / SDspH               0                0
        //  0                        0                        0                0
        //  -2 * CamX / SDspW - 1    2 * CamY / SDspH + 1     0                1

        float const sDspW = static_cast<float>(mDisplayPhysicalSize.width) * mZoomFactor;
        float const sDspH = static_cast<float>(mDisplayPhysicalSize.height) * mZoomFactor;

        // Recalculate Ortho Matrix cells (r, c)
        mOrthoMatrix[0][0] = 2.0f / sDspW;
        mOrthoMatrix[1][1] = -2.0f / sDspH;
        mOrthoMatrix[3][0] = -2.0f * (static_cast<float>(mCam.x) - mMarginWorkSize) / sDspW - 1.0f;
        mOrthoMatrix[3][1] = 2.0f * (static_cast<float>(mCam.y) - mMarginWorkSize) / sDspH + 1.0f;
    }

    static float CalculateZoomFactor(int zoom)
    {
        return std::ldexp(1.0f, -zoom);
    }

    int DisplayLogicalToWorkSpace(int size) const
    {
        return static_cast<int>(std::floor(static_cast<float>(size) * mZoomFactor));
    }

    float WorkSpaceToDisplayLogical(int size) const
    {
        return static_cast<float>(size) / mZoomFactor;
    }

private:

    // Constants
    static int constexpr MaxZoom = 6;
    static int constexpr MinZoom = -3;

    // Primary inputs
    int mZoom; // >=0: display pixels occupied by one work space pixel
    WorkSpaceCoordinates mCam; // Work space coordinates of of work pixel that is visible at (0, 0) in display
    int const mLogicalToPhysicalPixelFactor;
    DisplayLogicalSize mDisplayLogicalSize;
    DisplayPhysicalSize mDisplayPhysicalSize;

    // Calculated attributes
    float mZoomFactor; // DisplayPhysical is Work / ZoomFactor; ZoomFactor = # work pixels for 1 display pixel
    float mMarginWorkSize; // Work size of margin
    ProjectionMatrix mOrthoMatrix;
};

}