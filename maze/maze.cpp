#include "maze.h"









// return the current index in g_Maze
 int Maze::CellIdx()
{
	return Maze::g_PtX + Maze::NumCells * Maze::g_PtY;
}


int Maze::RandomInt()
{
	return static_cast<int>( dis( gen ) );
}

int Maze::RandomInt4()
{
	return static_cast<int>( dis4( gen ) );
}

////////////////////////////////////////////////////////////////////////////




////////////////////////////////////////////////////////////////////////////

bool Maze::IsDirValid( eDirection Dir )
{
	int NewX = Maze::g_PtX + Heading_X[ Dir ];
	int NewY = Maze::g_PtY + Heading_Y[ Dir ];

	if ( !Dir || NewX < 0 || NewY < 0 || NewX >= Maze::NumCells || NewY >= Maze::NumCells ) return false;

	return !g_Maze[ NewX + NumCells * NewY ];
}

Maze::eDirection Maze::GetDirection()
{
	eDirection Dir = eDirection( 1 << RandomInt4() );

	while ( true )
	{
		for ( int x = 0; x < 4; x++ )
		{
			if ( IsDirValid( Dir ) ) { return eDirection( Dir ); }

			Dir = eDirection( Dir << 1 );

			if ( Dir > eDirection_Left ) { Dir = eDirection_Up; }
		}

		Dir = eDirection( ( g_Maze[ CellIdx() ] & 0xf0 ) >> 4 );

		// nowhere to go
		if ( !Dir ) return eDirection_Invalid;

		g_PtX += Heading_X[ Dir ];
		g_PtY += Heading_Y[ Dir ];

		Dir = eDirection( 1 << RandomInt4() );
	}
}

void Maze::GenerateMaze()
{
	int Cells = 0;

	for ( eDirection Dir = GetDirection(); Dir != eDirection_Invalid; Dir = GetDirection() )
	{
		// a progress indicator, kind of
		if ( ++Cells % 1000 == 0 ) std::cout << ".";

		g_Maze[ CellIdx() ] |= Dir;

		g_PtX += Heading_X[ Dir ];
		g_PtY += Heading_Y[ Dir ];

		g_Maze[ CellIdx() ] = Mask[ Dir ];
	}



}

#if defined( __GNUC__ )
# define GCC_PACK(n) __attribute__((packed,aligned(n)))
#else
# define GCC_PACK(n) __declspec(align(n))
#endif // __GNUC__

#pragma pack(push, 1)
struct GCC_PACK( 1 ) sBMPHeader
{
	// BITMAPFILEHEADER
	unsigned short bfType;
	uint32_t bfSize;
	unsigned short bfReserved1;
	unsigned short bfReserved2;
	uint32_t bfOffBits;
	// BITMAPINFOHEADER
	uint32_t biSize;
	uint32_t biWidth;
	uint32_t biHeight;
	unsigned short biPlanes;
	unsigned short biBitCount;
	uint32_t biCompression;
	uint32_t biSizeImage;
	uint32_t biXPelsPerMeter;
	uint32_t biYPelsPerMeter;
	uint32_t biClrUsed;
	uint32_t biClrImportant;
};
#pragma pack(pop)

void Maze::SaveBMP( const char* FileName, const void* RawBGRImage, int Width, int Height )
{
	sBMPHeader Header;

	int ImageSize = Width * Height * 3;

	Header.bfType = 0x4D * 256 + 0x42;
	Header.bfSize = ImageSize + sizeof( sBMPHeader );
	Header.bfReserved1 = 0;
	Header.bfReserved2 = 0;
	Header.bfOffBits = 0x36;
	Header.biSize = 40;
	Header.biWidth = Width;
	Header.biHeight = Height;
	Header.biPlanes = 1;
	Header.biBitCount = 24;
	Header.biCompression = 0;
	Header.biSizeImage = ImageSize;
	Header.biXPelsPerMeter = 6000;
	Header.biYPelsPerMeter = 6000;
	Header.biClrUsed = 0;
	Header.biClrImportant = 0;

	std::ofstream File( FileName, std::ios::out | std::ios::binary );

	File.write( ( const char* )&Header, sizeof( Header ) );
	File.write( ( const char* )RawBGRImage, ImageSize );

	std::cout << "Saved " << FileName << std::endl;
}

void Maze::Line( unsigned char* img, int x1, int y1, int x2, int y2 )
{
	if ( x1 == x2 )
	{
		// vertical line
		for ( int y = y1; y < y2; y++ )
		{
			if ( x1 >= ImageSize || y >= ImageSize ) continue;
			int i = 3 * ( y * ImageSize + x1 );
			img[ i + 2 ] = img[ i + 1 ] = img[ i + 0 ] = 255;
		}
	}

	if ( y1 == y2 )
	{
		// horizontal line
		for ( int x = x1; x < x2; x++ )
		{
			if ( y1 >= ImageSize || x >= ImageSize ) continue;
			int i = 3 * ( y1 * ImageSize + x );
			img[ i + 2 ] = img[ i + 1 ] = img[ i + 0 ] = 255;
		}
	}
}

void Maze::RenderMaze( unsigned char* img )
{

	
	for ( int y = 0; y < NumCells; y++ )
	{
		for ( int x = 0; x < NumCells; x++ )
		{
			char v = g_Maze[ y * NumCells + x ];
			//std::cout<<std::hex<<v<<" ";
			int nx = x * CellSize;
			int ny = y * CellSize;

			if ( !( v & eDirection_Up    ) ) Line( img, nx,            ny,            nx + CellSize + 1, ny                );
			if ( !( v & eDirection_Right ) ) Line( img, nx + CellSize, ny,            nx + CellSize,     ny + CellSize + 1 );
			if ( !( v & eDirection_Down  ) ) Line( img, nx,            ny + CellSize, nx + CellSize + 1, ny + CellSize     );
			if ( !( v & eDirection_Left  ) ) Line( img, nx,            ny,            nx,                ny + CellSize + 1 );
		}
		//std::cout<<std::endl;
	}
}




