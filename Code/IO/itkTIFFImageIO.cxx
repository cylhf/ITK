/*=========================================================================


  Program:   Insight Segmentation & Registration Toolkit
  Module:    itkTIFFImageIO.cxx
  Language:  C++
  Date:      $Date$
  Version:   $Revision$

  Copyright (c) Insight Software Consortium. All rights reserved.
  See ITKCopyright.txt or http://www.itk.org/HTML/Copyright.htm for details.

     This software is distributed WITHOUT ANY WARRANTY; without even 
     the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR 
     PURPOSE.  See the above copyright notices for more information.

=========================================================================*/
#ifdef _MSC_VER
#pragma warning( disable : 4611 )
#endif 

#include "itkTIFFImageIO.h"
#include "itkRGBPixel.h"
#include "itkRGBAPixel.h"
#include <stdio.h>

#include <sys/stat.h>

extern "C" {
#include <tiffio.h>
}


namespace itk
{


class TIFFReaderInternal
{
public:
  TIFFReaderInternal();
  int Initialize();
  void Clean();
  int CanRead();
  int Open( const char *filename );
  TIFF *Image;
  unsigned int Width;
  unsigned int Height;
  unsigned short SamplesPerPixel;
  unsigned short Compression;
  unsigned short BitsPerSample;
  unsigned short Photometrics;
  unsigned short PlanarConfig;
  unsigned long int TileDepth;
};

int TIFFReaderInternal::Open( const char *filename )
{
  this->Clean();
  struct stat fs;
  if ( stat(filename, &fs) )
    {
    return 0;
    }
  this->Image = TIFFOpen(filename, "r");
  if ( !this->Image)
    {
    this->Clean();
    return 0;
    }
  if ( !this->Initialize() )
    {
    this->Clean();
    return 0;
    }
  return 1;
}

void TIFFReaderInternal::Clean()
{
  if ( this->Image )
    {
    TIFFClose(this->Image);
    }
  this->Image=NULL;
  this->Width = 0;
  this->Height = 0;
  this->SamplesPerPixel = 0;
  this->Compression = 0;
  this->BitsPerSample = 0;
  this->Photometrics = 0;
  this->PlanarConfig = 0;
  this->TileDepth = 0;
}

TIFFReaderInternal::TIFFReaderInternal()
{
  this->Image           = NULL;
  this->Clean();
}

int TIFFReaderInternal::Initialize()
{
  if ( this->Image )
    {
    if ( !TIFFGetField(this->Image, TIFFTAG_IMAGEWIDTH, &this->Width) ||
         !TIFFGetField(this->Image, TIFFTAG_IMAGELENGTH, &this->Height) )
      {
      return 0;
      }
    TIFFGetField(this->Image, TIFFTAG_SAMPLESPERPIXEL, 
                 &this->SamplesPerPixel);
    TIFFGetField(this->Image, TIFFTAG_COMPRESSION, &this->Compression);
    TIFFGetField(this->Image, TIFFTAG_BITSPERSAMPLE, 
                 &this->BitsPerSample);
    TIFFGetField(this->Image, TIFFTAG_PHOTOMETRIC, &this->Photometrics);
    TIFFGetField(this->Image, TIFFTAG_PLANARCONFIG, &this->PlanarConfig);
    if ( !TIFFGetField(this->Image, TIFFTAG_TILEDEPTH, &this->TileDepth) )
      {
      this->TileDepth = 0;
      }
    }
  return 1;
}

int TIFFReaderInternal::CanRead()
{
  return ( this->Image && ( this->Width > 0 ) && ( this->Height > 0 ) &&
           ( this->SamplesPerPixel > 0 ) && 
           ( this->Compression == COMPRESSION_NONE ) &&
           ( this->Photometrics == PHOTOMETRIC_RGB ||
             this->Photometrics == PHOTOMETRIC_MINISWHITE ||
             this->Photometrics == PHOTOMETRIC_MINISBLACK ||
             this->Photometrics == PHOTOMETRIC_PALETTE ) &&
           this->PlanarConfig == PLANARCONFIG_CONTIG &&
           ( !this->TileDepth ) &&
           ( this->BitsPerSample == 8 ) );
}




bool TIFFImageIO::CanReadFile(const char* file) 
{ 
  // First check the extension
  std::string filename = file;
  if(  filename == "" )
    {
    itkDebugMacro(<<"No filename specified.");
    return false;
    }

  bool extensionFound = false;
  std::string::size_type TIFFPos = filename.rfind(".TIFF");
  if ((TIFFPos != std::string::npos)
      && (TIFFPos == filename.length() - 5))
    {
    extensionFound = true;
    }

  TIFFPos = filename.rfind(".tiff");
  if ((TIFFPos != std::string::npos)
      && (TIFFPos == filename.length() - 5))
    {
    extensionFound = true;
    }

  TIFFPos = filename.rfind(".tif");
  if ((TIFFPos != std::string::npos)
      && (TIFFPos == filename.length() - 4))
    {
    extensionFound = true;
    }

  TIFFPos = filename.rfind(".TIF");
  if ((TIFFPos != std::string::npos)
      && (TIFFPos == filename.length() - 4))
    {
    extensionFound = true;
    }

  if( !extensionFound )
    {
    itkDebugMacro(<<"The filename extension is not recognized");
    return false;
    }

  // Now check if this is a valid TIFF image
  int res = m_InternalImage->Open(file);
  if (res)
    {
    return true;
    }
  m_InternalImage->Clean();
  return false;
}
  


void TIFFImageIO::ReadGenericImage( void *out, 
                                      unsigned int itkNotUsed(width), 
                                      unsigned int height )
{
  unsigned int isize = TIFFScanlineSize(m_InternalImage->Image);
  unsigned int cc;
  int row, inc;
  int xx=0, yy=0;
  tdata_t buf = _TIFFmalloc(isize);
  unsigned char *image = (unsigned char *)out;

  if ( m_InternalImage->PlanarConfig == PLANARCONFIG_CONTIG )
    {
    for ( row = 0; row < (int)height; row ++ )
      {
      if (TIFFReadScanline(m_InternalImage->Image, buf, row, 0) <= 0)
        {
        std::cout << "Problem reading the row: " << row << std::endl;
        break;
        }
      for (cc = 0; cc < isize; 
           cc += m_InternalImage->SamplesPerPixel )
        {
          inc = this->EvaluateImageAt( image, 
                                       static_cast<unsigned char *>(buf) +
                                       cc );      
          
          image += inc;
        xx++;
        }
      xx=0;
      yy++;
      }
    }
  else 
    {
    std::cout << "This reader can only do PLANARCONFIG_CONTIG" << std::endl;
    }

  _TIFFfree(buf); 
}

void TIFFImageIO::GetColor( int index, unsigned short *red, 
                                 unsigned short *green, unsigned short *blue )
{
  *red   = 0;
  *green = 0;
  *blue  = 0;
  if ( index < 0 ) 
    {
    std::cout << "Color index has to be greater than 0" << std::endl;
    return;
    }
  if ( this->TotalColors > 0 && 
       this->ColorRed && this->ColorGreen && this->ColorBlue )
    {
    if ( index >= this->TotalColors )
      {
       std::cout << "Color index has to be less than number of colors ("
                    << this->TotalColors << ")" << std::endl;
      return;
      }
    *red   = *(this->ColorRed   + index);
    *green = *(this->ColorGreen + index);
    *blue  = *(this->ColorBlue  + index);
    return;
    }

  unsigned short photometric;
  
  if (!TIFFGetField(m_InternalImage->Image, TIFFTAG_PHOTOMETRIC, &photometric)) 
    {
    if ( m_InternalImage->Photometrics != PHOTOMETRIC_PALETTE )
      {
      std::cout << "You can only access colors for palette images" << std::endl;
      return;
      }
    }
  
  unsigned short *red_orig, *green_orig, *blue_orig;
  
  switch (m_InternalImage->BitsPerSample) 
    {
    case 1: case 2: case 4:
    case 8: case 16:
        break;
    default:
      std::cout <<  "Sorry, can not image with " 
                     << m_InternalImage->BitsPerSample
                     << "-bit samples" << std::endl;
        return;
    }
  if (!TIFFGetField(m_InternalImage->Image, TIFFTAG_COLORMAP,
                    &red_orig, &green_orig, &blue_orig)) 
    {
    std::cout << "Missing required \"Colormap\" tag" << std::endl;
    return;
    }
  this->TotalColors = (1L << m_InternalImage->BitsPerSample);

  if ( index >= this->TotalColors )
    {
    std::cout << "Color index has to be less than number of colors ("
      << this->TotalColors << ")" << std::endl;
    return;
    }
  this->ColorRed   =   red_orig;
  this->ColorGreen = green_orig;
  this->ColorBlue  =  blue_orig;
  
  *red   = *(red_orig   + index);
  *green = *(green_orig + index);
  *blue  = *(blue_orig  + index);
}


int TIFFImageIO::EvaluateImageAt( void* out, void* in )
{
  unsigned char *image = (unsigned char *)out;
  unsigned char *source = (unsigned char *)in;
  int increment = 0;
  unsigned short red, green, blue, alpha;
  switch ( this->GetFormat() )
    {
    case TIFFImageIO::GRAYSCALE:
      if ( m_InternalImage->Photometrics == 
           PHOTOMETRIC_MINISBLACK )
        {
        *image = *source;
        }
      else
        {
        *image = ~( *source );
        }
      increment = 1;
      break;
    case TIFFImageIO::PALETTE_GRAYSCALE:
      this->GetColor(*source, &red, &green, &blue);
      *image = red;
      increment = 1;
      break;
    case TIFFImageIO::RGB_: 
      red   = *(source);
      green = *(source+1);
      blue  = *(source+2);
      *(image)   = red;
      *(image+1) = green;
      *(image+2) = blue;
      if ( m_InternalImage->SamplesPerPixel == 4 )
        {
        alpha = *(source+3);
        *(image+3) = 255-alpha;       
        }
      increment = m_InternalImage->SamplesPerPixel;
      break;
    case TIFFImageIO::PALETTE_RGB:
      this->GetColor(*source, &red, &green, &blue);     
      *(image)   = static_cast<unsigned char>(red >> 8);
      *(image+1) = static_cast<unsigned char>(green >> 8);
      *(image+2) = static_cast<unsigned char>(blue >> 8);
      increment = 3;
      break;
    default:
      return 0;
    }
  
  return increment;
}


unsigned int TIFFImageIO::GetFormat( )
{
  unsigned int cc;  

  if ( this->ImageFormat != TIFFImageIO::NOFORMAT )
    {
    return this->ImageFormat;
    }


  switch ( m_InternalImage->Photometrics )
    {
    case PHOTOMETRIC_RGB: 
    case PHOTOMETRIC_YCBCR: 
      this->ImageFormat = TIFFImageIO::RGB_;
      return this->ImageFormat;
    case PHOTOMETRIC_MINISWHITE:
    case PHOTOMETRIC_MINISBLACK:
      this->ImageFormat = TIFFImageIO::GRAYSCALE;
      return this->ImageFormat;
    case PHOTOMETRIC_PALETTE:
      for( cc=0; cc<256; cc++ ) 
        {
        unsigned short red, green, blue;
        this->GetColor( cc, &red, &green, &blue );
        if ( red != green || red != blue )
          {
          this->ImageFormat = TIFFImageIO::PALETTE_RGB;
          return this->ImageFormat;
          }
        }
      this->ImageFormat = TIFFImageIO::PALETTE_GRAYSCALE;
      return this->ImageFormat;
    }
  this->ImageFormat = TIFFImageIO::OTHER;
  return this->ImageFormat;
}




const std::type_info& TIFFImageIO::GetPixelType() const
{
  switch(m_PixelType)
    {
    case UCHAR:
      return typeid(unsigned char);
    case USHORT:
      return typeid(unsigned short);
    case CHAR:
      return typeid(char);
    case SHORT:
      return typeid(short);
    case UINT:
      return typeid(unsigned int);
    case INT:
      return typeid(int);
    case ULONG:
      return typeid(unsigned long);
    case LONG:
      return typeid(long);
    case FLOAT:
      return typeid(float);
    case DOUBLE:
      return typeid(double);
    case RGB:
      return typeid(RGBPixel<unsigned char>);
    case RGBA:
      return typeid(RGBAPixel<unsigned char>);
    default:
    {
    itkExceptionMacro ("Invalid type: " << m_PixelType << ", only unsigned char, unsigned short, RGB<unsigned char> are allowed.");
    return this->ConvertToTypeInfo(m_PixelType);      
    }
    case UNKNOWN:
      itkExceptionMacro ("Unknown pixel type: " << m_PixelType);
    }
  return typeid(ImageIOBase::UnknownType);
}

  
unsigned int TIFFImageIO::GetComponentSize() const
{
  switch(m_PixelType)
    {
    case UCHAR:
      return sizeof(unsigned char);
    case USHORT:
      return sizeof(unsigned short);
    case CHAR:
      return sizeof(char);
    case SHORT:
      return sizeof(short);
    case UINT:
      return sizeof(unsigned int);
    case INT:
      return sizeof(int);
    case ULONG:
      return sizeof(unsigned long);
    case LONG:
      return sizeof(long);
    case FLOAT:
      return sizeof(float);
    case DOUBLE:
      return sizeof(double);
    case RGB:
      return sizeof(unsigned char);
    case RGBA:
      return sizeof(unsigned char);
    case UNKNOWN:
    default:
    {
    itkExceptionMacro ("Invalid type: " << m_PixelType 
                       << ", only unsigned char and unsigned short are allowed.");
    return 0;
    }
    }
  return 1;
}

  
void TIFFImageIO::ReadVolume(void*)
{
  
}

  
void TIFFImageIO::Read(void* buffer)
{
  if ( m_InternalImage->Compression == COMPRESSION_OJPEG )
      {
      std::cout << "This reader cannot read old JPEG compression" << std::endl;
      return;
      }


  int width  = m_InternalImage->Width;
  int height = m_InternalImage->Height;



  if ( !m_InternalImage->CanRead() )
    {
    uint32 *tempImage 
      = static_cast<uint32*>( buffer );
    
      tempImage = new uint32[ width * height ];

    if ( !TIFFReadRGBAImage(m_InternalImage->Image, 
                            width, height, 
                            tempImage, 0 ) )
      {
      std::cout << "Problem reading RGB image" << std::endl;
      if ( tempImage != buffer )
        {
        delete [] tempImage;
        }
      
      return;
      }
    int xx, yy;
    unsigned char *simage = (unsigned char *)tempImage;
    uint32* ssimage = tempImage;
    unsigned char *fimage = (unsigned char *)buffer;
    for ( yy = 0; yy < height; yy ++ )
      {
      for ( xx = 0; xx < width; xx++ )
        {
        unsigned char red   = static_cast<unsigned char>(TIFFGetR(*ssimage));
        unsigned char green = static_cast<unsigned char>(TIFFGetG(*ssimage));
        unsigned char blue  = static_cast<unsigned char>(TIFFGetB(*ssimage));
        unsigned char alpha = static_cast<unsigned char>(TIFFGetA(*ssimage));
          
        *(fimage  ) = red;
        *(fimage+1) = green;
        *(fimage+2) = blue;
        *(fimage+3) = alpha;

        simage += 4;
        ssimage ++;
        }
      }
    
    if ( tempImage != 0 && tempImage != buffer )
      {
      delete [] tempImage;
      }
    return;
    }

  unsigned int format = this->GetFormat();  



  if ( m_InternalImage->Compression == COMPRESSION_PACKBITS )
    {
    height /= m_InternalImage->BitsPerSample;
    }
  
  switch ( format )
    {
    case TIFFImageIO::GRAYSCALE:
    case TIFFImageIO::RGB_: 
    case TIFFImageIO::PALETTE_RGB:
    case TIFFImageIO::PALETTE_GRAYSCALE:      
      this->ReadGenericImage( buffer, width, height );
      break;
    default:
      return;
    }



}


TIFFImageIO::TIFFImageIO()
{
  this->SetNumberOfDimensions(2);
  m_PixelType = UCHAR;
  
  this->InitializeColors();
  m_InternalImage = new TIFFReaderInternal;

  m_Spacing[0] = 1.0;
  m_Spacing[1] = 1.0;
  
  m_Origin[0] = 0.0;
  m_Origin[1] = 0.0;

  m_Compression = TIFFImageIO::PackBits;
}

TIFFImageIO::~TIFFImageIO()
{
  delete this->m_InternalImage;
}

void TIFFImageIO::PrintSelf(std::ostream& os, Indent indent) const
{
  Superclass::PrintSelf(os, indent);
  os << indent << "PixelType: " << m_PixelType << "\n";
  os << indent << "Compression: " << m_Compression << "\n";
}

void TIFFImageIO::InitializeColors()
{
  this->ColorRed    = 0;
  this->ColorGreen  = 0;
  this->ColorBlue   = 0;
  this->TotalColors = -1;  
  this->ImageFormat = TIFFImageIO::NOFORMAT;
}
  
void TIFFImageIO::ReadImageInformation()
{
  m_Spacing[0] = 1.0;  // We'll look for TIFF pixel size information later,
  m_Spacing[1] = 1.0;  // but set the defaults now

  m_Origin[0] = 0.0;
  m_Origin[1] = 0.0;


  int width  = m_InternalImage->Width;
  int height = m_InternalImage->Height;

  m_Dimensions[0] = width;
  m_Dimensions[1] = height;

 switch ( this->GetFormat() )
    {
    case TIFFImageIO::GRAYSCALE:
    case TIFFImageIO::PALETTE_GRAYSCALE:
      this->SetNumberOfComponents( 1 );
      break;
    case TIFFImageIO::RGB_:      
      this->SetNumberOfComponents( 
        m_InternalImage->SamplesPerPixel );
      break;
    case TIFFImageIO::PALETTE_RGB:      
      this->SetNumberOfComponents( 3 );
      break;
    default:
      this->SetNumberOfComponents( 4 );
    }

  if ( !m_InternalImage->CanRead() )
    {
    this->SetNumberOfComponents( 4 );
    }

  return;
}

bool TIFFImageIO::CanWriteFile( const char * name )
{
  std::string filename = name;

  if (filename == "")
    {
    return false;
    }
  
  std::string::size_type TIFFPos = filename.rfind(".TIFF");
  if ( (TIFFPos != std::string::npos)
       && (TIFFPos == filename.length() - 5) )
    {
    return true;
    }

  TIFFPos = filename.rfind(".tiff");
  if ( (TIFFPos != std::string::npos)
       && (TIFFPos == filename.length() - 5) )
    {
    return true;
    }

  TIFFPos = filename.rfind(".tif");
  if ( (TIFFPos != std::string::npos)
       && (TIFFPos == filename.length() - 4) )
    {
    return true;
    }


  TIFFPos = filename.rfind(".TIF");
  if ( (TIFFPos != std::string::npos)
       && (TIFFPos == filename.length() - 4) )
    {
    return true;
    }


  return false;
}


void TIFFImageIO::WriteImageInformation(void)
{
}

void TIFFImageIO::Write(const void* buffer)
{
  ImageIORegion ioRegion = this->GetIORegion();

  // Make sure the region to be written is 2D
  if ( ioRegion.GetRegionDimension() != 2 )
    {
    itkExceptionMacro(<<"TIFF Writer can only write 2-dimensional images");
    }
  
  this->WriteSlice(m_FileName, buffer);
}



class TIFFWriterIO
{
public:
  // Writing file no reading
  static tsize_t TIFFRead(thandle_t, tdata_t, tsize_t) { return 0; }

  // Write data
  static tsize_t TIFFWrite(thandle_t fd, tdata_t buf, tsize_t size) 
    {
    std::ostream *out = reinterpret_cast<std::ostream *>(fd);
    out->write(static_cast<char *>(buf), size);
    return out->fail() ? static_cast<tsize_t>(0) : size;
    }

  static toff_t TIFFSeek(thandle_t fd, toff_t off, int whence) 
    {
    std::ostream *out = reinterpret_cast<std::ostream *>(fd);
    switch (whence) 
      {
      case SEEK_SET:
        out->seekp(off, std::ios::beg);
        break;
      case SEEK_END:
        out->seekp(off, std::ios::end);
        break;
      case SEEK_CUR:
        out->seekp(off, std::ios::cur);
        break;
      default:
        return out->tellp();
      }
    return out->tellp();
    }

  // File will be closed by the superclass
  static int TIFFClose(thandle_t) { return 1; }

  static toff_t TIFFSize(thandle_t fd) 
    {
    std::ostream *out = reinterpret_cast<std::ostream *>(fd);
    out->seekp(0, std::ios::end);
    return out->tellp();
    }

  static int TIFFMapFile(thandle_t, tdata_t*, toff_t*) { return (0); }
  static void TIFFUnmapFile(thandle_t, tdata_t, toff_t) {}
};


void TIFFImageIO::WriteSlice(std::string& fileName, const void* buffer)
{
  const unsigned char *outPtr = ( (const unsigned char *) buffer);

  unsigned int width, height;
  width =  m_Dimensions[0];
  height = m_Dimensions[1];  

  int scomponents = this->GetNumberOfComponents();
  double resolution = -1;
  uint32 rowsperstrip = (uint32) -1;
  int bps;

    switch (this->GetComponentType())
    {
    case UCHAR:
      bps = 8;
      break;

    case USHORT:
      bps = 16;
      break;

    default:
      itkExceptionMacro(<<"TIFF supports unsigned char and unsigned short");
    }

  int predictor = 0;

  std::ofstream* file;

#ifdef _WIN32
  file = new std::ofstream(fileName.c_str(), std::ios::out | std::ios::binary);
#else
  file = new std::ofstream(fileName.c_str(), std::ios::out);
#endif

  TIFF* tif = TIFFClientOpen(fileName.c_str(), "w",
    (thandle_t) file,
    reinterpret_cast<TIFFReadWriteProc>(TIFFWriterIO::TIFFRead), 
    reinterpret_cast<TIFFReadWriteProc>(TIFFWriterIO::TIFFWrite),
    reinterpret_cast<TIFFSeekProc>(TIFFWriterIO::TIFFSeek),
    reinterpret_cast<TIFFCloseProc>(TIFFWriterIO::TIFFClose), 
    reinterpret_cast<TIFFSizeProc>(TIFFWriterIO::TIFFSize),
    reinterpret_cast<TIFFMapFileProc>(TIFFWriterIO::TIFFMapFile), 
    reinterpret_cast<TIFFUnmapFileProc>(TIFFWriterIO::TIFFUnmapFile)
    );

  if ( !tif )
    {
    return;
    }

  uint32 w = width;
  uint32 h = height;
  TIFFSetField(tif, TIFFTAG_IMAGEWIDTH, w);
  TIFFSetField(tif, TIFFTAG_IMAGELENGTH, h);
  TIFFSetField(tif, TIFFTAG_ORIENTATION, ORIENTATION_TOPLEFT);
  TIFFSetField(tif, TIFFTAG_SAMPLESPERPIXEL, scomponents);
  TIFFSetField(tif, TIFFTAG_BITSPERSAMPLE, bps); // Fix for stype
  TIFFSetField(tif, TIFFTAG_PLANARCONFIG, PLANARCONFIG_CONTIG);

  if ( scomponents > 3 )
    {
    // if number of scalar components is greater than 3, that means we assume
    // there is alpha.
    uint16 extra_samples = scomponents-3;
    uint16 *sample_info = new uint16[scomponents-3];
    sample_info[0]=EXTRASAMPLE_ASSOCALPHA;
    int cc;
    for ( cc = 1; cc < scomponents-3; cc ++ )
      {
      sample_info[cc] = EXTRASAMPLE_UNSPECIFIED;
      }
    TIFFSetField(tif,TIFFTAG_EXTRASAMPLES,extra_samples,
      sample_info);
    delete [] sample_info;
    }

  int compression = COMPRESSION_PACKBITS;
  switch ( m_Compression )
    {
  case TIFFImageIO::PackBits: compression = COMPRESSION_PACKBITS; break;
  case TIFFImageIO::JPEG:     compression = COMPRESSION_JPEG; break;
  case TIFFImageIO::Deflate:  compression = COMPRESSION_DEFLATE; break;
  case TIFFImageIO::LZW:      compression = COMPRESSION_LZW; break;
  default: compression = COMPRESSION_NONE;
    }
  //compression = COMPRESSION_JPEG;
  TIFFSetField(tif, TIFFTAG_COMPRESSION, compression); // Fix for compression
  uint16 photometric = PHOTOMETRIC_RGB;
  if ( compression == COMPRESSION_JPEG )
    {
    TIFFSetField(tif, TIFFTAG_JPEGQUALITY, 75); // Parameter
    TIFFSetField(tif, TIFFTAG_JPEGCOLORMODE, JPEGCOLORMODE_RGB);
    photometric = PHOTOMETRIC_YCBCR;
    }
  else if ( compression == COMPRESSION_LZW )
    {
    predictor = 2;
    TIFFSetField(tif, TIFFTAG_PREDICTOR, predictor);
    std::cout << "LZW compression is patented outside US so it is disabled" << std::endl;
    }
  else if ( compression == COMPRESSION_DEFLATE )
    {
    predictor = 2;
    TIFFSetField(tif, TIFFTAG_PREDICTOR, predictor);
    }

  TIFFSetField(tif, TIFFTAG_PHOTOMETRIC, photometric); // Fix for scomponents
  TIFFSetField(tif, TIFFTAG_ROWSPERSTRIP,
    TIFFDefaultStripSize(tif, rowsperstrip));
  if (resolution > 0) {
    TIFFSetField(tif, TIFFTAG_XRESOLUTION, resolution);
    TIFFSetField(tif, TIFFTAG_YRESOLUTION, resolution);
    TIFFSetField(tif, TIFFTAG_RESOLUTIONUNIT, RESUNIT_INCH);
  }




  int rowLength; // in bytes

  switch (this->GetComponentType())
    {
    case UCHAR:
      rowLength = sizeof(unsigned char); 
      break;
    case USHORT:
       rowLength = sizeof(unsigned short);
      break;
    default:
      itkExceptionMacro(<<"TIFF supports unsigned char and unsigned short");
    }

  rowLength *= this->GetNumberOfComponents();
  rowLength *= width;

  int row = 0;
  for (unsigned int idx2 = 0; idx2 < height; idx2++)
    {
       if ( TIFFWriteScanline(tif, const_cast<unsigned char*>(outPtr), row, 0) < 0)
        {
        std::cout << "TIFFImageIO: error out of disk space" << std::endl;
        break;
        }
      outPtr += rowLength;
      row ++;
    }

  TIFFClose(tif);
  file->close();

}


} // end namespace itk

