/*=========================================================================

  Program:   Insight Segmentation & Registration Toolkit
  Module:    itkImageSpatialObject.txx
  Language:  C++
  Date:      $Date$
  Version:   $Revision$

  Copyright (c) 2002 Insight Consortium. All rights reserved.
  See ITKCopyright.txt or http://www.itk.org/HTML/Copyright.htm for details.

     This software is distributed WITHOUT ANY WARRANTY; without even 
     the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR 
     PURPOSE.  See the above copyright notices for more information.

=========================================================================*/
#ifndef __ImageSpatialObject_txx
#define __ImageSpatialObject_txx


#include "itkImageSpatialObject.h"
#include "itkSize.h"

namespace itk
{

/** Constructor */
template< unsigned int NDimensions, class PixelType, unsigned int PipelineDimension >
ImageSpatialObject< NDimensions,  PixelType, PipelineDimension >
::ImageSpatialObject()
{
  strcpy(m_TypeName,"ImageSpatialObject");
  m_Image = ImageType::New();
  m_SlicePosition = new int[NDimensions];
  for(unsigned int i=0;i<NDimensions;i++)
  {
    m_SlicePosition[i]=0;
  }

  ComputeBounds();
}

/** Destructor */
template< unsigned int NDimensions, class PixelType, unsigned int PipelineDimension >
ImageSpatialObject< NDimensions,  PixelType, PipelineDimension >
::~ImageSpatialObject()
{
  delete m_SlicePosition;
}

/** Return true if the given point is inside the image */
template< unsigned int NDimensions, class PixelType, unsigned int PipelineDimension >
bool
ImageSpatialObject< NDimensions,  PixelType, PipelineDimension >
::IsEvaluableAt( const PointType & point )
{
  if( !IsInside( point ) )
  {
    return false;
  }
  return true; 
}

/** Return true if the given point is inside the image */
template< unsigned int NDimensions, class PixelType, unsigned int PipelineDimension >
bool
ImageSpatialObject< NDimensions,  PixelType, PipelineDimension >
::IsInside( const PointType & point ) const
{
  PointType p = point;
  TransformPointToLocalCoordinate( p );
  if(m_Bounds->IsInside( p ))
  {
    return true;
  }
  else
  {
    return Superclass::IsInside(p);
  }
}

/** Return the value of the image at a specified point 
 *  The value returned is always of type double */
template< unsigned int NDimensions, class PixelType, unsigned int PipelineDimension >
void 
ImageSpatialObject< NDimensions,  PixelType, PipelineDimension >
::ValueAt( const PointType & point, double & value )
{
  IndexType index;

  if( !IsEvaluableAt( point ) )
  {
    ExceptionObject e;
    e.SetLocation("ImageSpatialObject< NDimensions,  PixelType, PipelineDimension >::ValueAt( const PointType & )");
    e.SetDescription("the image value cannot be evaluated at the requested point");
    throw e;
  }

  PointType p = point;
  TransformPointToLocalCoordinate(p);
  if( m_Image->TransformPhysicalPointToIndex( p, index ) )
  {
    value = m_Image->GetPixel(index);
  }
}

/** Compute the bounds of the image */
template< unsigned int NDimensions, class PixelType, unsigned int PipelineDimension >
void
ImageSpatialObject< NDimensions,  PixelType, PipelineDimension >
::ComputeBounds( void )
{
  if( this->GetMTime() > m_BoundsMTime )
  { 
    typename ImageType::RegionType region = m_Image->GetLargestPossibleRegion();
    itk::Size<NDimensions> size = region.GetSize();
    PointType pointLow,pointHigh;
    typename VectorContainerType::Pointer points = VectorContainerType::New();
    points->Initialize(); 

    for( unsigned int i=0; i<NDimensions; i++ )
    {
      pointLow[i] = 0;
      pointHigh[i] = size[i];
    }
   
    points->InsertElement(0,pointLow);
    points->InsertElement(1,pointHigh);

    m_Bounds->SetPoints(points);
    m_Bounds->ComputeBoundingBox();
    m_BoundsMTime.Modified();
  }
}

/** Set the image in the spatial object */
template< unsigned int NDimensions, class PixelType, unsigned int PipelineDimension >
void
ImageSpatialObject< NDimensions,  PixelType, PipelineDimension >
::SetImage( ImageSpatialObject< NDimensions,  PixelType, PipelineDimension >::ImageType * image )
{
  m_Image = image;
  m_Image->Modified();
  ComputeBounds();
}

/** Get the image inside the spatial object */
template< unsigned int NDimensions, class PixelType, unsigned int PipelineDimension >
typename ImageSpatialObject< NDimensions,  PixelType, PipelineDimension >::ImageType *
ImageSpatialObject< NDimensions,  PixelType, PipelineDimension >
::GetImage( void )
{
  return m_Image.GetPointer();
}

/** Print the object */
template< unsigned int NDimensions, class PixelType, unsigned int PipelineDimension >
void
ImageSpatialObject< NDimensions,  PixelType, PipelineDimension >
::PrintSelf( std::ostream& os, Indent indent ) const
{
  Superclass::PrintSelf(os,indent);
  os << "Image: " << std::endl;
  os << indent << m_Image << std::endl;
}

/** Get the modification time */
template< unsigned int NDimensions, class PixelType, unsigned int PipelineDimension >
unsigned long 
ImageSpatialObject< NDimensions,  PixelType, PipelineDimension >
::GetMTime( void ) const
{
  unsigned long latestMTime = Object::GetMTime();
  unsigned long boundsMTime,imageMTime;
    
  if( (boundsMTime = m_Bounds->GetMTime()) > latestMTime )
  { 
    latestMTime = boundsMTime;
  }

  if( (imageMTime = m_Image->GetMTime()) > latestMTime )
  {
    latestMTime = imageMTime;
  }

  return latestMTime; 
}


/** Set the slice position */
template< unsigned int NDimensions, class PixelType, unsigned int PipelineDimension >
void
ImageSpatialObject< NDimensions,  PixelType, PipelineDimension >
::SetSlicePosition(unsigned int dimension, int position) 
{
  m_SlicePosition[dimension]=position;
  this->Modified();
}


} // end namespace itk

#endif //__ImageSpatialObject_txx
