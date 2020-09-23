
#if !defined(_ACFA_FILE_H_)
#define _ACFA_FILE_H_

#ifdef WIN32
#define HMAP std
#else
#define HMAP __gnu_cxx
#endif

#include <string>
#include <vector>
#include <map>
#ifdef WIN32
#include <hash_map>
#else
#include <ext/hash_map>
#endif
#include <deque>
#include <ostream>
#include <stdio.h>
#include <string.h>


#include "acfa_config.h"

namespace acfa {

	class ini_file;

	// pure virtual base class for different type of lines in ini file.
	class ini_basic_line{
	protected:
		char*	_key;
		char*	_value;
		char*	_delimiter;	// delimiter if used.

	public:
		ini_basic_line* _prev;		// pointer to previous line;
		ini_basic_line* _next;		// pointer to next line;

		ini_basic_line(void);
		virtual ~ini_basic_line();

		virtual const char* get_key(void) const { return _key;	 };
		virtual void set_key(char* s);

		virtual const char* get_value(void) const { return _value;	 };
		virtual void set_value(char* s);

		virtual void set_delimiter(char* d) { _delimiter = d;  };
		virtual const char* get_delimiter(void) const { return _delimiter; };

		//virtual const char* get_type(void) const = 0;	// get class name of this class;

		virtual int output(char* buf) = 0;	// output content into buffer, buf must larger than ACFA_MAX_LINE_LEN + 1
	};

	// a section, both get_key and get_value will return section name;
	class ini_section_line : public ini_basic_line{
	public:
		ini_section_line(void) : ini_basic_line(){};
		explicit ini_section_line(char* akey) : ini_basic_line(){ set_key(akey);	 };

		virtual const char* get_value(void) const	{ return NULL; };	// section do not have value;
		virtual void set_value(char* s)	{};

		virtual void set_delimiter(char* d) {  };						// we do not have delimiter;
		virtual const char* get_delimiter(void) const { return NULL; };

		//virtual const char* get_type(void) const { return t_line_type_section; };	// get class name of this class;

		virtual int output(char* buf);	// output content into buffer, buf must larger than ACFA_MAX_LINE_LEN + 1
	};

	// a key-value pair
	class ini_key_value_line : public ini_basic_line{
	public:
		ini_key_value_line(char* key, char* value);
		ini_key_value_line(void) : ini_basic_line(), _prev_same(NULL), _next_same(NULL), _hash(0) {};

		//virtual const char* get_type(void) const { return t_line_type_keyvalue; };	// get class name of this class;

		virtual int output(char* buf);	// output content into buffer, buf must larger than ACFA_MAX_LINE_LEN + 1

		virtual void set_key(char* s);	// update _hash in this func.

		ini_key_value_line *_prev_same;
		ini_key_value_line *_next_same;
        size_t _hash;
	};

	// a comment
	class ini_comment_line : public ini_basic_line{
	public:
		ini_comment_line(void) : ini_basic_line(){};

		virtual const char* get_key(void) const	{ return NULL; };		// comment line do not have key;
		virtual void set_key(char* s){};

		virtual void set_delimiter(char* d) {  };						// we do not have delimiter;
		virtual const char* get_delimiter(void) { return NULL; };

		//virtual const char* get_type(void) const { return t_line_type_comment; };	// get class name of this class;

		virtual int output(char* buf);	// output content into buffer, buf must larger than ACFA_MAX_LINE_LEN + 1
	};

	// a invalid line
	class ini_invalid_line : public ini_basic_line{
	public:
		ini_invalid_line(void) : ini_basic_line(){};

		virtual const char* get_key(void) const	  { return NULL; };		// invlaid line do not have key;
		virtual void set_key(char* s){};

		virtual void set_delimiter(char* d) {  };						// we do not have delimiter;
		virtual char* get_delimiter(void)	{ return NULL; };

		//virtual const char* get_type(void) const {	return t_line_type_invalid; };	// get class name of this class;

		virtual int output(char* buf);	// output content into buffer, buf must larger than ACFA_MAX_LINE_LEN + 1
	};

	// a spare line
	class ini_spare_line : public ini_basic_line{
	public:
		ini_spare_line(void) : ini_basic_line(){};

		virtual const char* get_key(void) const { return NULL; };	// spare line do not have key;
		virtual void set_key(char* s){};

		virtual void set_delimiter(char* d) {  };						// we do not have delimiter;
		virtual char* get_delimiter(void)	{ return NULL; };

		//virtual const char* get_type(void) const { return t_line_type_spare; };	// get class name of this class;

		virtual int output(char* buf);	// output content into buffer, buf must larger than ACFA_MAX_LINE_LEN + 1
	};

	// a include line;
	class ini_include_line : public ini_basic_line{
	public:
		ini_include_line(void) : ini_basic_line(){};

		virtual const char* get_key(void) const { return NULL; };		// include line do not have key
		virtual void set_key(char* s){};

		virtual void set_delimiter(char* d) {  };						// we do not have delimiter;
		virtual char* get_delimiter(void)	{ return NULL; };

		//virtual const char* get_type(void) const { return t_line_type_include; };	// get class name of this class;

		virtual int output(char* buf);	// output content into buffer, buf must larger than ACFA_MAX_LINE_LEN + 1
	};

	// manipulate all the lines in ini_file
	class ini_line_queue{
	public:
		ini_basic_line *_first;	// pointer to the first line
		ini_basic_line *_last;	// pointer to the last line

		ini_line_queue() : _first(NULL), _last(NULL){};
		~ini_line_queue();

		ini_basic_line* push_back(ini_basic_line* line);
		ini_basic_line* push_front(ini_basic_line* line);

		void erase(ini_basic_line* line);
		void clear(void);
		bool is_empty(void);

		void insert_before(ini_basic_line* line, ini_basic_line* pos, ini_basic_line* same = NULL);
		void insert_after(ini_basic_line* line, ini_basic_line* pos, ini_basic_line* same = NULL);
	};

	// section handle all the ini_line belongs to
	struct ini_section {
		//typedef std::deque<acfa::ini_key_value_line*>	t_key_value_list;
		typedef HMAP::hash_multimap<size_t, acfa::ini_key_value_line*> t_value_idx;

		ini_line_queue*		_lines;

		t_value_idx			_index;		// index to search in key-value
		ini_section_line*	_pos;		// pointer to the section line in ini file.
		ini_basic_line*		_last;		// pointer to the last line ini ini_file belong to it.

		explicit ini_section(ini_line_queue* q, ini_section_line* aLine = NULL)
			: _lines(q), _pos(aLine), _last(aLine), _prev(NULL), _next(NULL) {};

		void push_back(ini_basic_line *line);
		void erase(ini_key_value_line* line);

		t_value_idx::iterator find(const char* akey);							// find ,case sensitive
		t_value_idx::iterator ifind(const char* akey);						// find, NOT case sensitive

		void output(std::ostream& s, bool valueonly=true);	// output content to stream;

		ini_section* _prev;
		ini_section* _next;
	};

	struct my_compare_str{
		bool operator()(const char* a, const char* b) const {
#ifdef WIN32
			return _stricmp(a, b) < 0  ? true : false;
#else
			return strcasecmp(a, b)  < 0  ? true : false;
#endif
		}
	};

	struct ini_section_list{
		ini_section_list() : _first(NULL), _last(NULL) {};
		~ini_section_list();

		ini_section* _first;
		ini_section* _last;

		ini_section* append(ini_section*);
		void remove(ini_section*);
		void pop_front(void);
		void pop_back(void);
		bool is_empty(void)	{ return NULL == _first ? true : false; };
	};

	class ini_file{
	public:
		// index is combined by section name, key name, and order of key_value;
		typedef std::map<const char*, acfa::ini_section*, my_compare_str>	t_section_index;	// do not allow duplicate section.

	protected:
		std::string _name;		// file name
		std::string _path;      // file path
		std::string _fullname;  // full name include path and name;

		ini_line_queue	_lines;
		t_section_index _section_index;
		ini_section_list _sections;
		char* _comment_mark;

		virtual ini_section* _append_to_section(ini_section_line* sec_line);	// do not check duplicate
		//virtual void _delete_line(ini_section* sec, ini_basic_line* line);					// delete a line;

	public:
		explicit ini_file(char* fName = NULL, const char* mode = "r+b");
		virtual ~ini_file();

        std::string& get_name(void)     {   return _name;   };

		char* get_comment_mark(void)	{	return _comment_mark;	};
		void set_comment_mark(char* m) {	_comment_mark = m;	};

		int load(const char* fName, const char* mode = "r+b");			// load from file;
		int save(void);					// save file;
		int save_to(const char* fName, const char* mode = "w+b");		// save to new file;

		ini_basic_line* append_line(ini_basic_line* l);
		ini_section* find_section(const char* sec);
		ini_section* find_section(const ini_section_line* sec_line);
		ini_section_list& get_section_list(void)	{ return _sections; };
		ini_basic_line* get_first_line(void)	{ return _lines._first; };

		ini_key_value_line* find_first(char* section, char* key);	// get first line of the key;
		int get_duplicate_count(ini_key_value_line* line);			// get count of key-value that have same key

		const char* get_value(char* section, char* key);				// get value of a key;
		int get_value(char* section, char* key, std::string& v);	// get value of a key-value pair.
		int get_value_array(char* section, char* key, std::vector<std::string>& array);	// get a array of value;
		int	set_value(char* section, char* key, char* v, bool autocreate=true);	// set value of a key-value pair.
		int	update_value(char* section, char* key, char* oldv, char* newv);	    // update value of a key-value pair.
		int	set_value_array(char* section, char* key, std::vector<std::string>& array, bool autocreate=true);	// set value of a multiple key-value .
		void append_value(char* section, char*key, char*v, int delimiter=1);		// append value at the end of section;
		int insert_before(char* section, char* key, char* v, char* which);	// insert a value before which.
		int insert_after(char* section, char* key, char* v, char* which);	// insert a value after which.
		bool exist(char* section, char* key, char* v);					// check if value v already exist;
		int delete_key(char* section, char* akey);		// delete all lines with key = akey;
		int delete_value(char* section, char* akey, char* avalue, bool casesensitive=false);	// delete all lines with key = akey and value=avalue;

		bool delete_section(char* section);			// delete the complete section,
		bool rename_section(char* src, char* dst);	// rename the section src to dst

		int count(char* section, char* key);		// find with section and key, return number of key-value pair matched;

	};

}; // namespace acfa

#endif //_ACFA_FILE_H_
