#ifndef GCODE_HPP
#define GCODE_HPP

#include <cstring>
#include <string>
#include "device_c.hpp"
#include "types.h"

using namespace std;

#ifndef DEBUG_MSG
#define DEBUG_MSG
const string debug_msg = "\n"							\
     "Debug level goes from 0 (only show the most critical messages\n"	\
     "to 5 (output lots of extra debugging information). The default\n"	\
     "is 1 (only show error messages)\n";
#endif

// For historical reasons we use inches internally
const double MM_PER_INCH = 25.4;

enum debug_prio {
     crit = 0,
     err,
     warn,
     info,
     debug,
     extra_debug
};

// These are private utility classes
class line
{
     xy start, end;
     bool cut;
     enum debug_prio _debug;
     void debug_out(enum debug_prio, string);

public:
     line(const xy &, const xy &, const bool);
     ~line();
     xy draw(Device::Generic &);
};

class bezier
{
     // start, end and control points
     xy start, cp1, cp2, end;
     enum debug_prio _debug;
     void debug_out(enum debug_prio, string);

public:
     bezier(const xy &, const xy &, const xy &, const xy &);
     ~bezier();
     xy draw(Device::Generic &);
};

class arc
{
     // the g-code supplied values
     xy current, target;
     xy cvec;
     bool clockwise;
     const double k;

     // derived values defining the arc
     xy center;
     double radius, arcwidth, rotation;
     
     // we're implementing this as a 4-segment circle, so this is
     // appropriate
     bezier *segments[4];
     int cseg;

     // calculate the segments
     void segment_right(double rot);
     void segment(double swidth, double rot);

     enum debug_prio _debug;
     void debug_out(enum debug_prio, string);

     // utility - ideally this would be done as part of an xy class,
     // but I don't want to  make such a large change right now
     double angle_between(const xy &, const xy &);

public:
     arc(const xy &, const xy &, const xy &, const bool);
     arc(const xy &, double, double, double);
     ~arc();
     xy draw(Device::Generic &);
};

class gcode
{
     // parse methods
     double doc_to_internal(double);
     char get_command(const string &, size_t *);
     int get_code(const string &, size_t *);
     double get_value(const string &, size_t *);
     xy get_xy(const string &, size_t *);
     xy get_vector(const string, size_t *);
     xy get_target(const string, size_t *);

     // private stuff - methods so that they can access the private
     // methods and members
     void process_movement(string);
     void process_line(string);
     void process_clockwise_arc(string);
     void process_anticlockwise_arc(string);
     void process_g_code(string);
     void process_line_number(string);
     void process_parens(string);
     void process_misc_code(string);

     void debug_out(enum debug_prio, string);

     inline void raise_pen(void)
	  {
	       pen_up = true;
	  }
     inline void lower_pen(void)
	  {
	       pen_up = false;
	  }
     inline void set_metric(bool m)
	  {
	       metric = m;
	  }
     inline void set_absolute(bool a)
	  {
	       absolute = a;
	  }

     Device::Generic & cutter;
     std::string filename;
     xy curr_pos;

     bool pen_up;
     bool metric;
     bool absolute;

     // by default, only print critical stuff
     enum debug_prio _debug;

public:
     gcode(Device::Generic &);
     gcode( const  std::string &, Device::Generic & );
     ~gcode();
     
     void set_input(const std::string &);
     void set_cutter(Device::Generic &);

     void parse_file(void);
     void parse_line(string);

     inline bool is_pen_up(void)
	  {
	       return pen_up;
	  }
     inline bool is_metric(void)
	  {
	       return metric;
	  }
     inline bool is_absolute(void)
	  {
	       return absolute;
	  }
     inline void set_debug(enum debug_prio d)
	  {
	       _debug = d;
	  }

};
#endif
