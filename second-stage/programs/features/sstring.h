// sstring.h  -- Serializable strings
//
// Mark Johnson, 21st March 2005
//
// sstrings are a version of strings that are serializable, 
// i.e., have write/read invariance (blank spaces are escaped)

#ifndef SSTRING_H
#define SSTRING_H

#include <cctype>
#include <iostream>
#include <sstream>
#include <string>

#include "utility.h"


#define SSTRING_ESCAPE     '\\'
#define SSTRING_OPENQUOTE  '\"'
#define SSTRING_CLOSEQUOTE '\"'


template <typename CharT, typename Traits = std::char_traits<CharT>, typename Alloc = std::allocator<CharT> >
class basic_sstring : public std::basic_string<CharT, Traits, Alloc> { 

public:
  typedef typename std::basic_string<CharT, Traits, Alloc> basic_string;

  basic_sstring() { }

  template <typename T> 
  basic_sstring(const T& v) : basic_string(to_string(v)) { }

  basic_sstring(const CharT* str) : basic_string(str) { }
  basic_sstring(const basic_string& str) : basic_string(str) { }
  basic_sstring(const basic_sstring& str) : basic_string(str) { }

  template <typename T>
  static basic_string to_string(const T& v) {
    std::basic_ostringstream<CharT, Traits, Alloc> os;
    os << v;
    return os.str();
  }

  inline static bool dont_escape(char c) { 
    return isgraph(c) && c != '%' && c != '(' && c != ')' 
      && c != SSTRING_ESCAPE && c != SSTRING_OPENQUOTE && c != SSTRING_CLOSEQUOTE;
  }  // basic_sstring::dont_escape()

  inline static char escaped_char(char c) {
    switch (c) {
    case 'a': return('\a');
    case 'b': return('\b');
    case 'f': return('\f');
    case 'n': return('\n');
    case 'r': return('\r');
    case 't': return('\t');
    case 'v': return('\v');
    default: return c;
    }
    return c;
  }  // basic_sstring::escaped_char()

};

template <typename _CharT, typename _Traits, typename _Alloc>
std::basic_istream<_CharT, _Traits>& 
operator>> (std::basic_istream<_CharT, _Traits>& is, 
	    basic_sstring<_CharT, _Traits, _Alloc>& str) {
  str.clear();
  char c;
  if (!(is >> c)) return is;                         // If read fails, return error
  if (str.dont_escape(c) || c == SSTRING_ESCAPE) {   // Recognize a normal symbol
    do {
      if (c == SSTRING_ESCAPE) {
	if (!is.get(c)) return is;                   //  Read next character; return if read fails.
	str.push_back(str.escaped_char(c));          //  Push escaped char onto string.
      }
      else
	str.push_back(c);
    }
    while (is.get(c) && (str.dont_escape(c) || c == SSTRING_ESCAPE));
    if (!is.fail())                                  //  Did we read one too many chars?
      is.putback(c);                                 //   Yes.  Put it back.
    else if (is.eof())                               //  Are we at eof?
      is.clear(is.rdstate() & ~std::ios::failbit & ~std::ios::eofbit);
  }
  else if (c == SSTRING_OPENQUOTE) {                 // Recognize a quoted string
    if (!is.get(c)) return is;                       //  Read next character; return if read fails
    while (c != SSTRING_CLOSEQUOTE) {
      if (c == SSTRING_ESCAPE) {                     //  Is this character the escape character?
	if (!is.get(c)) return is;                   //   Yes.  Get quoted character.
	str.push_back(str.escaped_char(c));          //   Push character onto string.
      }
      else
	str.push_back(c);                            //   Push back ordinary character.
      if (!is.get(c)) return is;                     //  Read next character.
    }
  }
  else {                                             // c doesn't begin a symbol
    is.putback(c);                                   // put it back onto the stream
    is.clear(std::ios::failbit);                     // set the fail bit
  }
  return is;
} // operator>>(istream&, sstring&)


template <typename _CharT, typename _Traits, typename _Alloc>
std::basic_ostream<_CharT, _Traits>&
operator<<(std::basic_ostream<_CharT, _Traits>& os,
	   const basic_sstring<_CharT, _Traits, _Alloc>& str) {
  if (str.empty())
    os << SSTRING_OPENQUOTE << SSTRING_CLOSEQUOTE;
  else {
    bool needs_escaping = false;
    for (std::string::const_iterator si = str.begin(); si != str.end(); ++si)
      if (!str.dont_escape(*si)) {
	needs_escaping = true;
	break;
      }
    if (needs_escaping) {
      os.put(SSTRING_OPENQUOTE);
      for (std::string::const_iterator si = str.begin(); si != str.end(); ++si)  
	switch (*si) {
	case SSTRING_OPENQUOTE:
	  // case SSTRING_CLOSEQUOTE:
	case SSTRING_ESCAPE:
	  os.put(SSTRING_ESCAPE); os.put(*si); break;
	case '\a': os.put(SSTRING_ESCAPE); os.put('a'); break;
	case '\b': os.put(SSTRING_ESCAPE); os.put('b'); break;
	case '\f': os.put(SSTRING_ESCAPE); os.put('f'); break;
	case '\n': os.put(SSTRING_ESCAPE); os.put('n'); break;
	case '\r': os.put(SSTRING_ESCAPE); os.put('r'); break;
	case '\t': os.put(SSTRING_ESCAPE); os.put('t'); break;
	case '\v': os.put(SSTRING_ESCAPE); os.put('v'); break;
	default: os.put(*si); break;
	}
      os.put(SSTRING_CLOSEQUOTE);
    }
    else
      for (std::string::const_iterator si = str.begin(); si != str.end(); ++si)
	os << *si;
  }
  return os;
}  // operator<<(ostream&, sstring&)


typedef basic_sstring<char> sstring;

namespace EXT_NAMESPACE {

  // hash function for sstrings, copied from hash function for strings
  //
  template <typename CharT, typename Traits, typename Alloc>
  struct hash<basic_sstring<CharT, Traits, Alloc> > 
  {
    typedef basic_sstring<CharT, Traits, Alloc> basic_sstring_;

    size_t operator()(const basic_sstring_& s) const 
    {
      typedef typename basic_sstring_::const_iterator CI;
      
      unsigned long h = 0; 
      unsigned long g;
      CI p = s.begin();
      CI end = s.end();
      
      while (p!=end) {
	h = (h << 4) + (*p++);
	if ((g = h&0xf0000000)) {
	  h = h ^ (g >> 24);
	  h = h ^ g;
	}}
      return size_t(h);
    }  // hash<basic_sstring>::operator()
  };  // hash<basic_sstring>{}

}  // EXT_NAMESPACE{}

#endif // SSTRING_H
