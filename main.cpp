/*
  Copyright (c) 2020 anssi dot grohn at karelia fi

  This program is free software: you can redistribute it and/or modify
  it under the terms of the GNU General Public License as published by
  the Free Software Foundation, either version 3 of the License, or
  (at your option) any later version.
  
  This program is distributed in the hope that it will be useful,
  but WITHOUT ANY WARRANTY; without even the implied warranty of
  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
  GNU General Public License for more details.
  
  You should have received a copy of the GNU General Public License
  along with this program.  If not, see <https://www.gnu.org/licenses/>.
*/

#include <tinyxml2.h>
#include <nlohmann/json.hpp>
#include <iostream>
#include <tuple>
#include <vector>
#include <list>
#include <sstream>
using namespace std;
using namespace tinyxml2;
using json = nlohmann::json;
typedef tuple<double,double> Arc;
typedef vector<Arc> Arcs ;
typedef vector<Arcs> ArcsArray;
typedef vector<vector<int>> AreaArcIndices;
const double SCALE = 5000.0f;
struct ShapeMap
{
  Arcs arcs;
};

#define PI 3.14159265359
inline double RadiansToDegrees(double X)
{
  return X*57.29582790879777437539;
}
inline double ProjectToRange( double val,  double currentMin,  double currentMax,  double newMin,  double newMax)
{
  return (((val - currentMin) * (newMax - newMin)) / (currentMax - currentMin)) + newMin;
}

inline double Gudermannian( double yRadians )
{
    return RadiansToDegrees(atan(sinh(yRadians))); 
}

double GetLatitudeFromCartesian( float pixelY, float height )
{
  double yRadians = ProjectToRange(pixelY,  0.0,  height,  PI,  -PI);
  return Gudermannian(yRadians);
}

double GetLongitudeFromCartesian( float pixelX, float width)
{
    return ProjectToRange(pixelX,  0.0,  width,  -180.0,  180.0);
}

tuple<double,double> ToLatLong( const tuple<double,double> & coord, int width, int height )
{
  tuple<double,double> res;
  get<0>(res) = (GetLongitudeFromCartesian(get<0>(coord), width));
  get<1>(res) = (GetLatitudeFromCartesian(get<1>(coord), height));
  return res;
}

inline bool CloseTo( double a, double b, double epsilon = 0.001)
{
  return fabs(a-b) < epsilon;
}

// svg parse state
enum ParserState
{
   NONE,
   MOVETO,
   HLINE,
   VLINE,
   LINETO,
   CLOSEPATH,
   NUM_STATES
};

Arcs ParseArcs( const string & points )
{

  Arcs arcs;
  list<Arc> arcList;
  ////////////////////////////////////////////////////////////
  // parse individual coordinate points

  ParserState state = NONE;
  bool relativeCoords = false;

  // Values in viewbox coords.
  double currentx = 0.0f;
  double currenty = 0.0f;
  stringstream ss(points);
  string val;

  while ( !(ss >> val) == false )
  {
    Arc arc;
    stringstream ff(val);
    double x,y;


    // parse x and y, and if something goes wrong, we cannot insert pair
    if ( val == "m" )
    {
      state = MOVETO;
      relativeCoords = true;
      continue;
    }
    else if ( val == "M" )
    {
      // okay, LARGE M has only a single space-separated coordinate values, and no commas
      relativeCoords = false;
      
      // tokenize by comma, and attempt to parse double - continue if either fails.
      if ( !(ss >> val) || !(stringstream(val) >> x)) throw runtime_error("invalid x "+string(val));
      if ( !(ss >> val) || !(stringstream(val) >> y)) throw runtime_error("invalid y "+string(val));

      get<0>(arc) = x;
      get<1>(arc) = y;
      if ( relativeCoords )
      {
        get<0>(arc) += currentx;
        get<1>(arc) += currenty;
      }
      arcList.push_back(arc);
      currentx = get<0>(arc);
      currenty = get<1>(arc);
      continue;
    }
    else if ( val == "v" )
    {
      state = VLINE;
      relativeCoords = true;
      continue;
    }
    else if ( val == "V" )
    {
      state = VLINE;
      relativeCoords = false;
      continue;
    }
    else if ( val == "h" )
    {
      state = HLINE;
      relativeCoords = true;
      continue;
    }
    else if ( val == "H" )
    {
      state = HLINE;
      relativeCoords = false;
      continue;
    }
    else if ( val == "L" )
    {
      relativeCoords = false;
      // these are space-separated
      if ( !(ss >> val) || !(stringstream(val) >> x)) throw runtime_error("invalid x "+string(val));
      if ( !(ss >> val) || !(stringstream(val) >> y)) throw runtime_error("invalid y "+string(val));

      get<0>(arc) = x;
      get<1>(arc) = y;
      
      if ( relativeCoords )
      {
        get<0>(arc) += currentx;
        get<1>(arc) += currenty;
      }
      
      arcList.push_back(arc);
      currentx = get<0>(arc);
      currenty = get<1>(arc);
      continue;
    }
    else if ( val == "l" )
    {
      state = LINETO;
      relativeCoords = true;
      continue;
    }
    else if ( val == "z" || val == "Z" )
    {
      state = CLOSEPATH;
      // if end and initial start point of path differ, draw a line 
      if ( ! ( CloseTo(get<0>(arcList.front()), get<0>(arcList.back())) &&
               CloseTo(get<1>(arcList.front()), get<1>(arcList.back())) )  )
      {
        arcList.push_back(arcList.front());
      }
      // this should be the last command
      break;
    }
    else
    {
      switch ( state )
      {
      case VLINE:
        if ( !(stringstream(val) >> y)) throw runtime_error("invalid y "+string(val));
        x = get<0>(arcList.back());
        
        get<0>(arc) = x;
        get<1>(arc) = y;
        
        if ( relativeCoords )
        {
          get<0>(arc) += currentx;
          get<1>(arc) += currenty;
        }
        
        arcList.push_back(arc);
        state = LINETO;
        currentx = get<0>(arc);
        currenty = get<1>(arc);
        break;
      case HLINE:
        if ( !(stringstream(val) >> x)) throw runtime_error("invalid x "+string(val));
        y = get<1>(arcList.back());
        get<0>(arc) = x;
        get<1>(arc) = y;
        if ( relativeCoords )
        {
          get<0>(arc) += currentx;
          get<1>(arc) += currenty;
        }
        arcList.push_back(arc);
        currentx = get<0>(arc);
        currenty = get<1>(arc);
        state = LINETO;
        break;
      case MOVETO:
        {
          // tokenize by comma, and attempt to parse double - continue if either fails.
          if ( !getline(ff,val,',') || !(stringstream(val) >> x)) throw runtime_error("invalid x "+string(val));
          if ( !getline(ff,val,',') || !(stringstream(val) >> y)) throw runtime_error("invalid y "+string(val));

          get<0>(arc) = x;
          get<1>(arc) = y;
          if ( relativeCoords )
          {
            get<0>(arc) += currentx;
            get<1>(arc) += currenty;
          }
          arcList.push_back(arc);
          currentx = get<0>(arc);
          currenty = get<1>(arc);
        }
        state = LINETO;
        break;
      case LINETO:
        // tokenize by comma, and attempt to parse double - continue if either fails.
        if ( !getline(ff,val,',') || !(stringstream(val) >> x)) throw runtime_error("invalid x "+string(val));
        if ( !getline(ff,val,',') || !(stringstream(val) >> y)) throw runtime_error("invalid y "+string(val));


        get<0>(arc) = x;
        get<1>(arc) = y;
        
        if ( relativeCoords )
        {
          get<0>(arc) += currentx;
          get<1>(arc) += currenty;
        }

        arcList.push_back(arc);
        currentx = get<0>(arc);
        currenty = get<1>(arc);
        break;
      }
    }
  }

  arcs.insert(arcs.end(), arcList.begin(), arcList.end());

  return arcs;
}

struct Viewbox
{
  double x0;
  double y0;
  double x1;
  double y1;

};

// Conversion tool for SVG polygons to topojson.
// assumes svg viewBox coordinates beginning from 0,0. 
int main(int argc, char **argv )
{
  
  double width = 0.0f, height = 0.0f;
  ArcsArray allArcs;
  json topo;
  topo["type"] ="Topology";
  // in transform property exists, we would need to quantize coords as well
  // (convert to ints with scale, and turn into delta encoded ones.)

  topo["objects"] =  {
                      { "map",
                        {
                         {"type", "GeometryCollection"}
                        }
                      }
  };
  topo["objects"]["map"]["geometries"] = json::array();
  XMLDocument doc;
  if ( doc.LoadFile( argv[1] ) == XML_SUCCESS )
  {
    XMLElement * elem = doc.FirstChildElement("svg");
    const XMLAttribute * a = elem->FindAttribute("width");
    width = a->DoubleValue();
    a = elem->FindAttribute("height");
    height = a->DoubleValue();

    a = elem->FindAttribute("viewBox");
    string viewBox = a->Value();
    Viewbox vb;
    stringstream ss(viewBox);
    ss >> vb.x0;
    ss >> vb.y0;
    ss >> vb.x1;
    ss >> vb.y1;

    // start path processing
    elem = elem->FirstChildElement("path");
    if ( elem == nullptr )
    {
      cerr << "no path element found, exiting.\n";
      return 1;
    }
    do
    {
      json area;
      const XMLAttribute *attrib = elem->FindAttribute("d");
      string d;
      if ( attrib) d = attrib->Value();
      attrib = elem->FindAttribute("id");
      string id;
      if ( attrib ) id = attrib->Value();

      Arcs arcs = ParseArcs(d);
      
      allArcs.push_back(arcs);
      // each arc in area is an index to a arc array in all arcs array?
      // Yeah, go figure the property naming in this one...
      vector<vector<vector<size_t>>> arcIndicesDef  = { { { allArcs.size()-1 } } };
      area["arcs"] = arcIndicesDef;
      area["type"] = "MultiPolygon";
      string name = string("name_")+id;
      string iso = string("iso_")+id;
      area["properties"] = {
                            { "ID",  id },
                            { "name", name },
                            { "ISO", iso}
      };
      //cout << "area" << area.dump(4) << "\n";
      
      topo["objects"]["map"]["geometries"].push_back(area);  
      
    } while ( elem = elem->NextSiblingElement("path"));
    
    // do actual coordinate conversion following mercator projection
    for ( auto & v : allArcs )
    {
      for ( auto & a : v )
      {

         a = ToLatLong(a,vb.x1,vb.y1);
      }
    }
#ifdef QUANTIZE    
    // quantize values (delta-encode)
    for ( auto & v : allArcs )
    {
      for ( int i=v.size()-1;i>=0;i-- ) 
      {
        
        if ( i == 0 )
        {
          get<0>(v[i]) *= SCALE;
          get<1>(v[i]) *= SCALE;
        }
        else
        {
          get<0>(v[i]) = (int)((get<0>(v[i]) - get<0>(v[i-1]))*SCALE);
          get<1>(v[i]) = (int)((get<1>(v[i]) - get<1>(v[i-1]))*SCALE);
        }
      }
    }
    topo["transform"] = {
                       {"scale", { 1.0/SCALE, 1.0/SCALE} },
                       {"translate", {0.0, 0.0} }
    };
#endif
    topo["arcs"] = allArcs;
  }
  else
  {
    cerr << "Looks like I can't read that file.\n";
    return 1;
  }
  
  cout << topo.dump() << "\n";
  
  return 0;
}
