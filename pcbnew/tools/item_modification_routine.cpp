/*
 * This program source code file is part of KiCad, a free EDA CAD application.
 *
 * Copyright (C) 2023 KiCad Developers, see AUTHORS.txt for contributors.
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * as published by the Free Software Foundation; either version 2
 * of the License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, you may find one here:
 * http://www.gnu.org/licenses/old-licenses/gpl-2.0.html
 * or you may search the http://www.gnu.org website for the version 2 license,
 * or you may write to the Free Software Foundation, Inc.,
 * 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301, USA
 */

#include "item_modification_routine.h"

namespace
{

/**
 * Check if two segments share an endpoint (can be at either end of either segment)
 */
bool SegmentsShareEndpoint( const SEG& aSegA, const SEG& aSegB )
{
    return ( aSegA.A == aSegB.A || aSegA.A == aSegB.B || aSegA.B == aSegB.A || aSegA.B == aSegB.B );
}

} // namespace

wxString LINE_FILLET_ROUTINE::GetCommitDescription() const
{
    return _( "Fillet Lines" );
}

wxString LINE_FILLET_ROUTINE::GetCompleteFailureMessage() const
{
    return _( "Unable to fillet the selected lines." );
}

wxString LINE_FILLET_ROUTINE::GetSomeFailuresMessage() const
{
    return _( "Some of the lines could not be filleted." );
}

void LINE_FILLET_ROUTINE::ProcessLinePair( PCB_SHAPE& aLineA, PCB_SHAPE& aLineB )
{
    if( aLineA.GetLength() == 0.0 || aLineB.GetLength() == 0.0 )
        return;

    SEG       seg_a( aLineA.GetStart(), aLineA.GetEnd() );
    SEG       seg_b( aLineB.GetStart(), aLineB.GetEnd() );
    VECTOR2I* a_pt;
    VECTOR2I* b_pt;

    if( seg_a.A == seg_b.A )
    {
        a_pt = &seg_a.A;
        b_pt = &seg_b.A;
    }
    else if( seg_a.A == seg_b.B )
    {
        a_pt = &seg_a.A;
        b_pt = &seg_b.B;
    }
    else if( seg_a.B == seg_b.A )
    {
        a_pt = &seg_a.B;
        b_pt = &seg_b.A;
    }
    else if( seg_a.B == seg_b.B )
    {
        a_pt = &seg_a.B;
        b_pt = &seg_b.B;
    }
    else
        // Nothing to do
        return;


    SHAPE_ARC sArc( seg_a, seg_b, m_filletRadiusIU );
    VECTOR2I  t1newPoint, t2newPoint;

    auto setIfPointOnSeg = []( VECTOR2I& aPointToSet, SEG aSegment, VECTOR2I aVecToTest )
    {
        VECTOR2I segToVec = aSegment.NearestPoint( aVecToTest ) - aVecToTest;

        // Find out if we are on the segment (minimum precision)
        if( segToVec.EuclideanNorm() < SHAPE_ARC::MIN_PRECISION_IU )
        {
            aPointToSet.x = aVecToTest.x;
            aPointToSet.y = aVecToTest.y;
            return true;
        }

        return false;
    };

    //Do not draw a fillet if the end points of the arc are not within the track segments
    if( !setIfPointOnSeg( t1newPoint, seg_a, sArc.GetP0() )
        && !setIfPointOnSeg( t2newPoint, seg_b, sArc.GetP0() ) )
    {
        AddFailure();
        return;
    }

    if( !setIfPointOnSeg( t1newPoint, seg_a, sArc.GetP1() )
        && !setIfPointOnSeg( t2newPoint, seg_b, sArc.GetP1() ) )
    {
        AddFailure();
        return;
    }

    auto tArc = std::make_unique<PCB_SHAPE>( GetBoard(), SHAPE_T::ARC );

    tArc->SetArcGeometry( sArc.GetP0(), sArc.GetArcMid(), sArc.GetP1() );

    // Copy properties from one of the source lines
    tArc->SetWidth( aLineA.GetWidth() );
    tArc->SetLayer( aLineA.GetLayer() );
    tArc->SetLocked( aLineA.IsLocked() );

    MarkItemModified( aLineA );
    MarkItemModified( aLineB );

    AddNewItem( std::move( tArc ) );

    *a_pt = t1newPoint;
    *b_pt = t2newPoint;
    aLineA.SetStart( seg_a.A );
    aLineA.SetEnd( seg_a.B );
    aLineB.SetStart( seg_b.A );
    aLineB.SetEnd( seg_b.B );

    AddSuccess();
}

wxString LINE_CHAMFER_ROUTINE::GetCommitDescription() const
{
    return _( "Chamfer Lines" );
}

wxString LINE_CHAMFER_ROUTINE::GetCompleteFailureMessage() const
{
    return _( "Unable to chamfer the selected lines." );
}

wxString LINE_CHAMFER_ROUTINE::GetSomeFailuresMessage() const
{
    return _( "Some of the lines could not be chamfered." );
}

void LINE_CHAMFER_ROUTINE::ProcessLinePair( PCB_SHAPE& aLineA, PCB_SHAPE& aLineB )
{
    if( aLineA.GetLength() == 0.0 || aLineB.GetLength() == 0.0 )
        return;

    SEG seg_a( aLineA.GetStart(), aLineA.GetEnd() );
    SEG seg_b( aLineB.GetStart(), aLineB.GetEnd() );

    // If the segments share an endpoint, we won't try to chamfer them
    // (we could extend to the intersection point, but this gets complicated
    // and inconsistent when you select more than two lines)
    if( !SegmentsShareEndpoint( seg_a, seg_b ) )
    {
        // not an error, lots of lines in a 2+ line selection will not intersect
        return;
    }

    std::optional<CHAMFER_RESULT> chamfer_result =
            ComputeChamferPoints( seg_a, seg_b, m_chamferParams );

    if( !chamfer_result )
    {
        AddFailure();
        return;
    }

    auto tSegment = std::make_unique<PCB_SHAPE>( GetBoard(), SHAPE_T::SEGMENT );

    tSegment->SetStart( chamfer_result->m_chamfer.A );
    tSegment->SetEnd( chamfer_result->m_chamfer.B );

    // Copy properties from one of the source lines
    tSegment->SetWidth( aLineA.GetWidth() );
    tSegment->SetLayer( aLineA.GetLayer() );
    tSegment->SetLocked( aLineA.IsLocked() );

    AddNewItem( std::move( tSegment ) );

    MarkItemModified( aLineA );
    MarkItemModified( aLineB );

    // Shorten the original lines
    aLineA.SetStart( chamfer_result->m_updated_seg_a->A );
    aLineA.SetEnd( chamfer_result->m_updated_seg_a->B );
    aLineB.SetStart( chamfer_result->m_updated_seg_b->A );
    aLineB.SetEnd( chamfer_result->m_updated_seg_b->B );

    AddSuccess();
}


wxString LINE_EXTENSION_ROUTINE::GetCommitDescription() const
{
    return _( "Extend Lines to Meet" );
}

wxString LINE_EXTENSION_ROUTINE::GetCompleteFailureMessage() const
{
    return _( "Unable to extend the selected lines to meet." );
}

wxString LINE_EXTENSION_ROUTINE::GetSomeFailuresMessage() const
{
    return _( "Some of the lines could not be extended to meet." );
}

void LINE_EXTENSION_ROUTINE::ProcessLinePair( PCB_SHAPE& aLineA, PCB_SHAPE& aLineB )
{
    if( aLineA.GetLength() == 0.0 || aLineB.GetLength() == 0.0 )
        return;

    SEG seg_a( aLineA.GetStart(), aLineA.GetEnd() );
    SEG seg_b( aLineB.GetStart(), aLineB.GetEnd() );

    if( seg_a.Intersects( seg_b ) )
    {
        // already intersecting, nothing to do
        return;
    }

    OPT_VECTOR2I intersection = seg_a.IntersectLines( seg_b );

    if( !intersection )
    {
        // This might be an error, but it's also possible that the lines are
        // parallel and don't intersect.  We'll just ignore this case.
        return;
    }

    const auto line_extender = [&]( const SEG& aSeg, PCB_SHAPE& aLine )
    {
        // If the intersection point is not already n the line, we'll extend to it
        if( !aSeg.Contains( *intersection ) )
        {
            const int dist_start = ( *intersection - aSeg.A ).EuclideanNorm();
            const int dist_end = ( *intersection - aSeg.B ).EuclideanNorm();

            const VECTOR2I& furthest_pt = ( dist_start < dist_end ) ? aSeg.B : aSeg.A;

            MarkItemModified( aLine );
            aLine.SetStart( furthest_pt );
            aLine.SetEnd( *intersection );
        }
    };

    line_extender( seg_a, aLineA );
    line_extender( seg_b, aLineB );

    AddSuccess();
}