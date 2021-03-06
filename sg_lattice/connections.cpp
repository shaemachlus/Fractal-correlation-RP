//
// Created by Shae Machlus on 6/10/2019
// Successfully determined neighbors for arbitrary n, s, and dum on 6/25/2019
//
///////////////////////////////////////////////////
//SIERPENSKI GASKET LATTICE CONNECTIONS GENERATOR//
///////////////////////////////////////////////////
//

#include <iostream>
#include <cmath>
#include <vector>
#include <algorithm>
#include <fstream>
#include <numeric>
#include "sg_connections.h"

using namespace std;

//Calculates the total number of vertices in the system
int TotalVerts(int n, int s)
{
    int var = 0;
    
    for(int k = 0; k <= n; k++)
    {
        var += pow(3,k);
    }
    
    int size = pow(s,2) * var;
    return size;
}

//Calculates the number of vertices along the edge of a strip of the lattice
int EdgeVerts(int n, int s)
{
    int m = 2;
    int M;
    
    //Recursive formula
    for(int i = 0; i < n; i++)
    {
        int old_m = m;
        m = 2 * old_m - 1;
        if(i + 1 == n)
        {
            M = m;
        }
    }
    
    int len = s * (M - 1);
    return len;
}

//Compares "sum" and "size"--they should be equal
void SizeCheck(vector<int> vec, int size)
{
    int sum = 0;
    
    for (int j = 0; j < vec.size(); j++)
    {
        sum += vec[j];
    }
    cout << "Total number of vertices = " << sum << endl;
    cout << "Size of the system       = " << size << endl;
}

//Generates the number of vertices for each layer of an n-iterated SG
//(the formula was derived by observing and
//extrapolating pattern of vertex increase by hand)
void SG_VertList(vector<int> &verts, int n)
{
    int k = 1;
    
    for (int imax = 1; imax < pow(2,n) - 1; k++, imax = pow(2,k) - 1)
    {
        for (int i = 0; i < imax; i++)
        {
            verts.push_back(2 * verts[i + 1]);
            if (i + 1 == imax)
            {
                int endval = verts.back();
                verts.push_back(1 + endval);
            }
            
        }
    }
}

//Multiplies each layer by the number of SG's to obtain
//the number of vertices in each periodic strip of the lattice
void SG_StripVertList(vector<int> &verts, int s)
{
    for (int j = 0; j < verts.size(); j++)
    {
        verts[j] = s * verts[j];
    
        if (j + 1 == verts.size()) //if we're at the last layer
        {
            verts[j] = verts[j] - (2 * s) + s; //subtract off all of the corners and add the proper number of corners, note, with PBC
        }
    }
}

//Adds more vertices to the list corresponding to the additional s-1 strips,
//therefore creating a vertex list for the entire lattice
void SG_LatticeVertList(vector<int> &verts, int s)
{
    //The number of layers in the top strip of the lattice
    //(where both the top AND bottom points are all included, except the right corner of the furthest right SG)
    int strip_size = int(verts.size());
    
    for (int i = 0; i < s - 1; i++)
    {
        //Copy/pastes the info from the first strip into the list agian, but skipping the 0th layer
        //so as to not double count big triangle corner points
        for (int j = 1; j < strip_size; j++)
        {
            verts.push_back(verts[j]);
        }
        
        //At the very bottom of the lattice,
        //the big triangle corners should be removed for PBC
        if (i + 1 == s - 1)
        {
            verts.back() = verts.back() - s;
        }
    }
}


//Assigns elements of the layers vector.
//The purpose of this vector is that, given an arbitrary vertex number from 0 to size - 1,
//you can access the layer of the lattice it's in
void FillLayers(vector<int> &verts, vector<int> &lays)
{
    int count = 0;
    for (int j = 0; j < verts.size(); j++)
    {
        for(int i = 0; i < verts[j]; i++)
        {
            lays[i + count] = j;
        }
        count += verts[j];
    }
}

//For an arbitrary vertex index dummy, this function assigns its vertex type (1, 2, 3, or 4) in the types vector
void TypeAssign(vector<int> &type, int dummy, int dum_layer, int numer, int denom, int next_layer_verts, int len, int s)
{
    //Covers type identification of odd numbered layers
    if((dum_layer + 1) % 2 == 0)
    {
        if(numer % 2 == 0)
        {
            type[dummy] = 2;
            return;
        }
        else
        {
            type[dummy] = 3;
            return;
        }
    }
    
    //Covers type identification of the 0th layer
    if(dum_layer == 0)
    {
        type[dummy] = 1;
        return;
    }
    
    //Covers type identification of the bottom layer
    if(dum_layer == len)
    {
        type[dummy] = 4;
        return;
    }
    
    //Covers type identification of the even, non special (not top or bottom) layers
    if(dum_layer % 2 == 0 && dum_layer != len && dum_layer != 0)
    {
        //if the next layer has a vertex which belongs to a different triangle, then the layer is special and should be given a 1 4 4 ...
        if(next_layer_verts == 2)
        {
            if(numer == 1)
            {
                type[dummy] = 1;
                return;
            }
            else
            {
                type[dummy] = 4;
                return;
            }
        }
        // Layer is not special, it is either a 342, 342, ..., layer, or a 34444..., 2, 34444..., 2 ..., layer.
        else
        {
            //When the layer is divisible by 4, it is a a 34444..., 2, 3444..., 2, ..., layer.
            
            //next_layer_verts/4 is the number of equal parts that the  3444..., 2 layer should be split into
            if(dum_layer % 4 == 0)
            {
                int segs = next_layer_verts/4;
                int vps = denom/segs;
                
                if((numer - 1) % vps == 0)
                {
                    type[dummy] = 3;
                    return;
                }
                if(numer % vps == 0)
                {
                    type[dummy] = 2;
                    return;
                }
                else
                {
                    type[dummy] = 4;
                    return;
                }
            }
            
            //The last possible case. If you've gotten here, DUM is in a 342, 342, ... layer.
            else
            {
                switch ((numer - 1) % 3)
                {
                    case 0:
                        type[dummy] = 3;
                        break;
                    case 1:
                        type[dummy] = 4;
                        break;
                    case 2:
                        type[dummy] = 2;
                        break;
                }
            }
        }
    }
}

//Calculates depth of index dum into its home layer, dum_layer
//By PBC, behind the 0th layer is the last layer.
int Depth(vector<int> &lays, int dum, int dum_layer, int size)
{
    int j = dum;
    int track = 0;
    
    do
    {
        if(j == 0)
        {
            j = size;
        }
        j -= 1;
        track++;
    }
    while(lays[j] == dum_layer);
    
    return track;
}

// given all the information about the site, returns its neighbor in the deisred direction
int DoEverything(int dum, int n, int s, int size, int d)
{
    int ll = EdgeVerts(n, s);                          //The number of vertices along one edge of a lattice strip
    int n1; int n2; int n3; int n4; int n5; int n6;    //Neighbor variables
    int tri_col = 0;                                       //The column of the parallelogram unit cell which dum is in
    
    //File for output
    ofstream connects;
    connects.open("mathematica_lines.txt", ios::app);
    
    //Each element slot # corresponds to a different layer.
    //The element itself is the number of vertices on that layer.
    //Any lattice will at least have three layers.
    vector<int> vertices(3);
    
    //The 0th layer at least has one vertex (for one triangle)
    //The 1st layer at least has two vertices (for one triangle)
    vertices[0] = 1;
    vertices[1] = 2;
    vertices[2] = 3;
    
    //Each element slot # is the name of a different vertex.
    //The element itself is the layer that vertex can be found in.
    vector<int> layers(size);
    
    //Each element slot # is the name of a different vertex.
    //The element itself is the type of vertex that each vertex is (defined by its connectivity).
    vector<int> type(size);
    
    //Fills vector vertices
    SG_VertList(vertices, n);
    
    //Expands vector vertices to include a whole strip of the lattice
    SG_StripVertList(vertices, s);
    
    //Expands vector vertices to account for each vertex in the lattice
    SG_LatticeVertList(vertices, s);
    
    //Fills vector layers
    FillLayers(vertices, layers);
    
    //The layer dum is in
    int test_layer = layers[dum];
    
    //The potential top neighbors of dum
    vector<int> bachelors(0);
    
    //The potential bottom neighbors of dum
    vector<int> bachelorettes(0);
    
    //The other vertices on the same layer and triangle as dum
    vector<int> buddies(0);
    
    //The number of vertices per triangle in test_layer, its below layer, and its above layer
    int vert_count = vertices[test_layer]/s;
    int vert_count_below = vertices[test_layer + 1]/s;
    int vert_count_above;
    
    if(test_layer != 0)
    {
        vert_count_above = vertices[test_layer - 1]/s;
    }
    
    //How many indicies back you need to go back from dum before you hit a different layer
    int track = Depth(layers, dum, test_layer, size);
    
    //Fills tri_col
    do
    {
        tri_col++;
    }while(tri_col * vert_count < track);
    
    //The index of dum is index 1 is the first vertex on dum's layer and dum's triangle
    int single_tri_index = track - (tri_col - 1) * vert_count;
    
    //The fractional location of dum relative to its buddies on the layer and triangle
    double dum_frac =  single_tri_index/(double)vert_count;
    
    //Assigns the vertex type to dum
    TypeAssign(type, dum, test_layer, single_tri_index, vert_count, vert_count_below, ll, s);
    
    ///////////////////////////////
    //Generic Neighbor Assignment//
    ///////////////////////////////
    
    ///////////////////
    //Above Neighbors//
    ///////////////////
    
    if(test_layer != 0)
    {
        //On the same triangle as dum, urev, is the upper right edge vertex (where upper is just the layer above dum)
        int urev = dum - track - s * vert_count_above + tri_col * vert_count_above;
        
        //This loop fills the bachelors vector with all of the above neighbors that are capable of "reaching down"
        //To have neighbors in the layer below. I.e., all of the above neighbors on dum's triangle which are not type 4.
        for(int i = 0; i < vert_count_above; i++)
        {
            int track3 = Depth(layers, urev - i, test_layer - 1, size);
            int r = 0;
            do
            {
                r++;
            }while(r * vertices[layers[urev]] / s < track3);
            int sup_tri_index = track3 - (r - 1) * vert_count_above;
            
            TypeAssign(type, urev - i, test_layer - 1, sup_tri_index, vert_count_above, vert_count, ll, s);
            
            if(type[urev - i] != 4)
            {
                bachelors.push_back(urev - i);
            }
        }
        
        reverse(bachelors.begin(),bachelors.end());
        double x1 = double(vert_count)/bachelors.size();

        //Neighbor assignment based on a pattern found by hand
        if(test_layer != ll)
        {
            if(x1 == 2.0 && single_tri_index % 2 == 0)
            {
                n6 = bachelors[single_tri_index/2 - 1];
                n1 = n6 + 1;
            }
            else if(x1 == 2.0 && single_tri_index % 2 == 1)
            {
                n1 = bachelors[(single_tri_index - 1)/2];
                n6 = n1 - 1;
            }
            else if(test_layer % (ll/s) == 0)
            {
                n1 = bachelors[single_tri_index - 1];
                n6 = n1 - 1;
            }
            else
            {
                int num = vert_count / gcd(vert_count, int(bachelors.size()));
                int v = (single_tri_index - 1) / num;
                n1 = bachelors[single_tri_index - 1 - v];
                n6 = n1 - 1;
            }
            
            //Ends
            if(single_tri_index == 1)
            {
                //no n6 is single_tri_index is 1
                n1 = bachelors[0];
            }
            
            if(dum_frac == 1 && test_layer % (ll/s) != 0)
            {
                //no n1 if dum_frac is 1
                n6 = bachelors[bachelors.size() - 1];
            }
        }
        
        
        if(test_layer == ll)
        {
            n6 = bachelors[single_tri_index - 1];
            n1 = n6 + 1;
        }
    }
    
    ///////////////////
    //Below Neighbors//
    ///////////////////
    
    if(test_layer != ll && type[dum] != 4)
    {
        //On the same triangle as dum, brev, is the bottom right edge vertex (where bottom is just the layer below dum)
        int brev = dum - track + s * vert_count + tri_col * vert_count_below;
        
        if((test_layer + 1) % (ll/s) == 0 && test_layer != ll)
        {
            brev++;
        }
        
        int M;
        
        //Fills buddies with all of the vertices on the same layer as dum that aren't type 4
        for(int i = 0; i < vert_count; i++)
        {
            int track2 = Depth(layers, dum - single_tri_index + 1 + i, test_layer, size);
            int r = 0;
            do
            {
                r++;
            }while(r * vertices[layers[dum - single_tri_index + 1 + i]] / s < track2);
            int tri_index = track2 - (r - 1) * vert_count;
            
            TypeAssign(type, dum - single_tri_index + 1 + i, test_layer, tri_index, vert_count, vert_count_below, ll, s);
            
            if(type[dum - single_tri_index + 1 + i] != 4)
            {
                buddies.push_back(dum - single_tri_index + 1 + i);
                
                //Makes note of where dum is located within buddies
                if(dum - single_tri_index + 1 + i == dum)
                {
                    M = int(buddies.size() - 1);
                }
            }
        }
        
        //Fills bachelorettes with all of the vertices in the layer below, which are the potential "connections" that dum can be paired with
        if((test_layer + 1) % (ll/s) == 0 && test_layer != ll)
        {
            for(int i = 0; i <= vert_count_below; i++)
            {
                bachelorettes.push_back(brev - i);
            }
        }
        else
        {
            for(int i = 0; i < vert_count_below; i++)
            {
                bachelorettes.push_back(brev - i);
            }
        }
        
        reverse(bachelorettes.begin(),bachelorettes.end());
        double x2 = double(bachelorettes.size())/buddies.size();
   
        //The actual neighbor assignment based on a pattern found by hand
        if(test_layer != ll)
        {
            //Generic assignment (not general enough for n = 5)
            if(x2 == 2.0)
            {
                n4 = bachelorettes[2 * M];
            }
            else if (vert_count + 1 == vert_count_below)
            {
                n4 = bachelorettes[M];
            }
            else
            {
                int den = double(buddies.size()) / int(gcd(bachelorettes.size(), buddies.size()));
                int w = M/den;
                n4 = bachelorettes[M + w];
            }
            
            //Bottom layer of a big triangle
            if(test_layer % (ll/s) == 0)
            {
                n4 = bachelorettes[M];
            }
                
            //The inner lattice big triangle verticies
            if(test_layer % (ll/s) == 0 && single_tri_index == 1 && test_layer != 0)
            {
                n4 = bachelorettes[0] - 2;
            }
            
            //When n4 is in the bottom layer
            if(test_layer == ll - 1)
            {
                n4 = bachelorettes[M] - 1;
            }
                
            n3 = n4 + 1;
        }
    }
    
    ////////////////////////////
    //Left and Right Neighbors//
    ////////////////////////////
    
    n2 = dum + 1;
    n5 = dum - 1;
    
    ////////////////////////////////////////////////////////////////////
    //Neighbor Assignment of Unconnected Points around Type 1 vertices//
    ////////////////////////////////////////////////////////////////////
    
    //Points left, above type 1 vertex
    if(dum_frac == 1 && test_layer % (ll/s) == ll/s - 1)
    {
        n3 = dum - track + 1 + vertices[test_layer] + vert_count_below * tri_col;
    }
    
    //Points right, below type 1 vertex
    if(dum_frac == 1 && test_layer % (ll/s) == 1)
    {
        n6 = dum - track + 1 - vert_count_above * (s - tri_col);
    }
    
    //Points left, below type 1 vertex
    if(dum_frac == 0.5 && test_layer % (ll/s) == 1)
    {
        n1 = dum - track + 1 - vert_count_above * (s - tri_col);
    }
    
    /////////////////////////////////////////
    //Neighbor Assignment of Special Points//
    /////////////////////////////////////////
    
    //Points left of right edges
    if(test_layer != layers[dum + 1] && test_layer % (ll/s) == 0)
    {
        n2 = dum - track + 1;
    }
    
    //Left edge points
    if(dum != 0)
    {
        if(test_layer != layers[dum - 1] && test_layer % (ll/s) == 0 && test_layer != ll)
        {
            n5 = dum + ll - 1;
            n6 = dum - 1;
            n3 = dum + ll + s * vert_count_below - 1;
            n4 = n3 - 1;
        }
    }
    
    //Points left, above right edges
    if(test_layer + 1 == layers[dum + 1] && test_layer % (ll/s) == ll/s - 1)
    {
        n3 = dum + 1;
    }
    
    //Points right, below right edges
    if(test_layer + 1 == layers[dum + 1] && test_layer % (ll/s) == 1)
    {
        n6 = dum - track - vertices[test_layer - 1] + 1;
    }
    
    //Points left, below right edges
    //Breakdown point of n = 0 configuration
    if(test_layer + 1 == layers[dum + 2] && test_layer % (ll/s) == 1 && test_layer == layers[dum + 1])
    {
        n1 = dum - track - vertices[test_layer - 1] + 1;
    }
    
    //////////////////////////////////////////////
    //Neighbor Assignment of VERY Special Points//
    //////////////////////////////////////////////
    
    //Top points that aren't the corner
    if(dum < s - 1)
    {
        n1 = size - vertices[ll] - (s - tri_col) * vertices[ll - 1]/s;
        n6 = n1 - 1;
        n2 = size - (s - tri_col) * vertices[ll]/s;
        n5 = n2 - 1;
    }
    //The corner of the unit cell
    if(dum == s - 1)
    {
        n1 = size - vertices[ll] - vertices[ll - 1];
        n2 = size - vertices[ll];
        n5 = size - 1;
        n6 = n2 - 1;
    }
    
    //Bottom points
    for(int m = 0; m < s; m++)
    {
        //Right of identified points
        if(dum == size - vertices[ll] + m * (ll - s)/s)
        {
            if(tri_col == 1)
            {
                n5 = s - 1;
            }
            else
            {
                n5 = tri_col - 2;
            }
        }
        //Left of identified points
        if(dum == size - vertices[ll] + (ll - s)/s - 1 + m * (ll - s)/s)
        {
            if(tri_col == s)
            {
                n2 = s - 1;
            }
            else
            {
                n2 = tri_col - 1;
            }
        }
    }
    
    //Second to bottom points
    for(int m = 0; m < s; m++)
    {
        //Right, above identified points
        if(dum == size - vertices[ll] - vertices[ll - 1] + m * vertices[ll - 1]/s)
        {
            if(tri_col == 1)
            {
                n4 = s - 1;
            }
            else
            {
                n4 = (tri_col - 2);
            }
        }
        //Left, above identified points
        if(dum == size - vertices[ll] - vertices[ll - 1] + vertices[ll - 1]/s - 1 + m * vertices[ll - 1]/s)
        {
            if(tri_col == s)
            {
                n3 = s - 1;
            }
            else
            {
                n3 = tri_col - 1;
            }
        }
    }
    //Second to top points
    for(int m = 0; m < 2 * s; m++)
    {
        if(test_layer == 1)
        {
            if(type[dum] == 3)
            {
                n1 = tri_col - 1;
            }
            if(type[dum] == 2)
            {
                n6 = tri_col - 1;
            }
        }
    }
    
    //Ensures vertex type is respected
    switch (type[dum])
    {
            //All six neighbors
        case 1:
            break;
            
            //No top right neighbor, no right neighbor
        case 2:
            n1 = n2 = -1;
            break;
            
            //No top left neighbor, no left neighbor
        case 3:
            n5 = n6 = -1;
            break;
            
            //No bottom right neighbor, no bottom left neighbor
        case 4:
            n3 = n4 = -1;
            break;
    }
    
    if(n1 != -1)
    {
        connects << dum + 1 << " " <<  n1 + 1 << endl;
    }
    if(n2 != -1)
    {
        connects << dum + 1 << " " <<  n2 + 1 << endl;
    }
    if(n3 != -1)
    {
        connects << dum + 1 << " " <<  n3 + 1 << endl;
    }
    if(n4 != -1)
    {
        connects << dum + 1 << " " <<  n4 + 1 << endl;
    }
    if(n5 != -1)
    {
        connects << dum + 1 << " " <<  n5 + 1 << endl;
    }
    if(n6 != -1)
    {
        connects << dum + 1 << " " <<  n6 + 1 << endl;
    }
    
    //cout << dum << " " << n1 << " " << n2 << " " << n3 << " " << n4 << " " << n5 << " " << n6 << endl;
    
    vertices.clear();
    layers.clear();
    type.clear();
    bachelors.clear();
    bachelorettes.clear();
    buddies.clear();
    
    switch (d) {
        case 1:
            return n1;
            break;
        case 2:
            return n2;
            break;
        case 3:
            return n3;
            break;
        case 4:
            return n4;
            break;
        case 5:
            return n5;
            break;
        default:
            return n6;
            break;
    }
}
