/* ------------------------------------------------------------------   */
/*      item            : OpenALXMLReader.cxx
        made by         : Rene' van Paassen
        date            : 230201
        category        : body file
        description     :
        changes         : 230201 first version
        language        : C++
        copyright       : (c) 23 TUDelft-AE-C&S
*/

#include "OpenALListener.hxx"
#include "OpenALXMLReader.hxx"
#include "OpenALObject.hxx"
#include <pugixml.hpp>
#include <dueca/debug.h>
#include <exception>

namespace worldlistener {

struct error_reading_openal_xml : public std::exception
{
  const char *what() { return "Cannot read XML file"; }
};

inline void ltrim(std::string &s)
{
  s.erase(s.begin(), std::find_if(s.begin(), s.end(),
                                  [](int ch) { return !std::isspace(ch); }));
}

  // trim from end (in place)
inline void rtrim(std::string &s)
{
  s.erase(
    std::find_if(s.rbegin(), s.rend(), [](int ch) { return !std::isspace(ch); })
      .base(),
    s.end());
}

inline std::string trim_copy(std::string s)
{
  ltrim(s);
  rtrim(s);
  return s;
}

inline std::vector<double> getValues(const std::string &s)
{
  std::stringstream invals(s);
  std::vector<double> res;
  double tmp;
  while (invals >> tmp) {
    res.push_back(tmp);
    if (invals.peek() == ',') {
      invals.ignore();
    }
  }
  return res;
}

OpenALXMLReader::CoordinateMapping::CoordinateMapping(unsigned offset,
                                                      unsigned size) :
  offset(offset),
  size(size)
{}

bool OpenALXMLReader::ObjectCoordinateMapping::getMapping(
  unsigned &offset, unsigned &size, const std::string &cname)
{
    // if no coordinate names given, assume this simply fills the array
  if (!cname.size()) {
    return true;
  }

  const auto idx = mappings.find(cname);
  if (idx == mappings.end()) {
    return false;
  }
  offset = idx->second.offset;
  size = idx->second.size;
  return true;
}

OpenALXMLReader::OpenALXMLReader(const std::string &definitions)
{
    // shortcut exit
  if (!definitions.size()) {
    W_MOD("Empty definitions file for reader!");
    return;
  }

    // read the coordinate definitions
  pugi::xml_document doc;
  auto result = doc.load_file(definitions.c_str());

  if (result) {
      // basic element is maps
    auto _maps = doc.child("maps");

      // run over all defined types
    for (auto _type = _maps.child("type"); _type;
         _type = _type.next_sibling("type")) {
      auto nm = object_mappings.emplace(_type.attribute("name").value(),
                                        ObjectCoordinateMapping());

      for (auto _coord = _type.child("param"); _coord;
           _coord = _coord.next_sibling("param")) {
        unsigned offset = _coord.attribute("offset").as_uint();
        unsigned nelts = _coord.attribute("size").as_uint(1U);
        std::string name = trim_copy(_coord.child_value());
        nm.first->second.mappings.emplace(std::piecewise_construct,
                                          std::forward_as_tuple(name),
                                          std::forward_as_tuple(offset, nelts));
      }
    }
  }
  else {
    throw error_reading_openal_xml();
  }
}

bool OpenALXMLReader::readWorld(const std::string &file, OpenALListener &viewer)
{
  pugi::xml_document doc;
  auto result = doc.load_file(file.c_str());
  if (!result) {
    W_MOD("Cannot read openal world from " << file);
    return false;
  }

    // get the container
  auto world = doc.child("world");

    // each declaration gets translated in data for a createable object
    // either through direct creation or from a channel entry
  for (auto def = world.child("declaration"); def;
       def = def.next_sibling("declaration")) {

      // Prepare the data for the object
    WorldDataSpec spec;

      // required stuff is key and type
    auto _key = def.attribute("key");
    auto _type = def.attribute("type");
    auto _name = def.attribute("name");

      // test required attributes are there
    if (!_key || !_type) {
      W_MOD("Skipping template, missing key and type");
      continue;
    }

      // store the data
    spec.type = _type.value();
    if (_name) {
      spec.name = _name.value();
    }

      // first word from type
    auto idxs = spec.type.find(" ");
    std::string tname =
      (idxs == std::string::npos) ? spec.type : spec.type.substr(0, idxs);

    // get all the files/string data
    for (auto fname = def.child("file"); fname;
         fname = fname.next_sibling("file")) {
      spec.filename.push_back(trim_copy(fname.child_value()));
    }

      // now get&translate all coordinates
    for (auto coord = def.child("param"); coord;
         coord = coord.next_sibling("param")) {
      std::string _label = coord.attribute("name").value();
      auto values = getValues(coord.child_value());
      unsigned offset = 0;
      unsigned n = values.size();
      auto idx = object_mappings.find(tname);

      if (idx != object_mappings.end()) {
        if (!idx->second.getMapping(offset, n, _label)) {
          W_MOD("Param index '" << _label << "' for type '" << tname
                                << "' missing");
          continue;
        }
      }
      else if (_label.size()) {
        W_MOD("Param mappings for type '" << tname << "' missing");
        continue;
      }
      spec.setCoordinates(offset, n, values);
    }

    viewer.addFactorySpec(_key.value(), spec);
  }

  // process the static sounds
  for (auto sta = world.child("static"); sta;
       sta = sta.next_sibling("static")) {

    auto name = sta.attribute("name");
    auto type = sta.attribute("type");
    WorldDataSpec spec;

    // overwrite with the type if specified
    spec.type = type.value();

    // first word from type
    auto idxs = spec.type.find(" ");
    std::string tname =
      (idxs == std::string::npos) ? spec.type : spec.type.substr(0, idxs);

    // name / parent from default (template, none) or overwritten
    if (name) {
      spec.name = name.value();
    }

      // run the files, overwrite all if available
    for (auto fname = sta.child("file"); fname;
         fname = fname.next_sibling("file")) {
      spec.filename.push_back(trim_copy(fname.child_value()));
    }

      // run the coordinates, overwrite or modify
    for (auto coord = sta.child("param"); coord;
         coord = coord.next_sibling("param")) {
      std::string _label = coord.attribute("name").value();
      auto values = getValues(coord.child_value());
      unsigned offset = 0;
      unsigned n = values.size();
      auto idx = object_mappings.find(tname);

      if (idx != object_mappings.end()) {
        if (!idx->second.getMapping(offset, n, _label)) {
          W_MOD("Param index '" << _label << "' for type '" << tname
                                << "' missing");
          continue;
        }
      }
      else if (_label.size()) {
        W_MOD("Param mappings for type '" << tname << "' missing");
        continue;
      }
      spec.setCoordinates(offset, n, values);
    }

    // create the object
    boost::intrusive_ptr<OpenALObject> nob(new OpenALObject(spec));
    viewer.addConstantSource(nob);
  }
  return true;
}

}; // namespace worldlistener
