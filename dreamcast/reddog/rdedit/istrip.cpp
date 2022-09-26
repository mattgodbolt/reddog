/*
 * Test implementation file for the IStrip stripifier
 */

#define MATT_DEBUG 1
#include "IStrip.h"

// Test
class MyOutput : public IOutput<IStripVertex>
{
public:
	void MidStrip (const IStripVertex &v)
	{
		cout << v << " ";
	}
	void EndStrip ()
	{ cout << endl; }
};

int main(int argc, char *argv[])
{
	typedef IStripPoly<IStripVertex> Poly;
#if 1
	// Test data: a cube
	IStripVertex vert[8] = {
		IStripVertex(0,  0,  0),
		IStripVertex(10, 0,  0),
		IStripVertex( 0,10,  0),
		IStripVertex(10,10,  0),
		IStripVertex(0,  0, 10),
		IStripVertex(10, 0, 10),
		IStripVertex( 0,10, 10),
		IStripVertex(10,10, 10)
	};
	int faces[12][3] = {
		{ 0, 2, 3 },
		{ 3, 1, 0 },
		{ 4, 5, 7 },
		{ 7, 6, 4 },
		{ 0, 1, 5 },
		{ 5, 4, 0 },
		{ 6, 7, 3 },
		{ 3, 2, 6 },
		{ 1, 3, 7 },
		{ 7, 5, 1 },
		{ 4, 6, 2 },
		{ 2, 0, 4 }
	};
#else
	IStripVertex vert[] = {
		IStripVertex(0,  0,  0),
		IStripVertex(10,10,  0),
		IStripVertex( 0,10,  0),
		IStripVertex( 0,20,  0),
		IStripVertex(10,20,  0)
	};
	int faces[][3] = {
		{ 0, 1, 2 },
		{ 2, 1, 3 },
		{ 3, 1, 4 }
	};
#endif
	vector<Poly> polygons;

	for (int i = 0; i < 2; ++i)
		polygons.push_back(Poly(vert[faces[i][0]], vert[faces[i][1]], vert[faces[i][2]]));

	Stripifier<IStripVertex> s(polygons);

//	s.GenerateHighOrderPolygons();

	s.Stripify();

	int strips, verts, swaps, polys;
	s.getMetrics (strips, verts, swaps, polys);
	cout << "Encoded " << polys << " triangles using " << strips << " strips " <<
		" and " << swaps << " swaps."<< endl;
	cout << "Average strip length    = " << float(verts)/float(strips) << endl;
	cout << "Average polys per strip = " << float(polys)/float(strips) << endl;

	MyOutput foo;
	s.Output (&foo);

	return 0;
}
