/*******************************************************************\

Module: SMT2 to Java Conversion

Author: Peter Schrammel

\*******************************************************************/

#include <solvers/smt2/smt2_parser.h>

#include <solvers/smt2/smt2_format.h>

#include <fstream>
#include <iostream>

#include <util/format.h>
#include <util/format_expr.h>
#include <util/message.h>
#include <util/namespace.h>
#include <util/replace_symbol.h>
#include <util/simplify_expr.h>
#include <util/string_utils.h>
#include <util/symbol_table.h>

#include <java_bytecode/expr2java.h>

#include <solvers/flattening/boolbv.h>
#include <solvers/sat/satcheck.h>

class smt2tojavat : public smt2_parsert
{
public:
  smt2tojavat(std::istream &_in) : smt2_parsert(_in)
  {
  }

protected:
  void command(const std::string &) override;
  void define_constants();
  void expand_function_applications(exprt &);

  std::set<irep_idt> constants_done;
};

void smt2tojavat::define_constants()
{
  symbol_tablet symbol_table;
  namespacet ns(symbol_table);
  for(const auto &id : id_map)
  {
    const irep_idt &identifier = id.first;

    // already done?
    if(constants_done.find(identifier) != constants_done.end())
    {
      continue;
    }

    typet type = id.second.type;

    // TODO: unsound hack for unsigned types
    if(type.id() == ID_unsignedbv)
      type = signedbv_typet(to_bitvector_type(type).get_width());

    if(id.second.definition.is_nil())
    {
      std::string java_type_string = type2java(type, ns);
      std::cout << "    " << java_type_string << " " << identifier << " = ";
      if(java_type_string == "boolean")
        std::cout << "CProver.nondetBoolean()";
      else if(java_type_string == "char")
        std::cout << "CProver.nondetChar()";
      else if(java_type_string == "byte")
        std::cout << "CProver.nondetByte()";
      else if(java_type_string == "int")
        std::cout << "CProver.nondetInt()";
      else if(java_type_string == "short")
        std::cout << "CProver.nondetShort()";
      else if(java_type_string == "long")
        std::cout << "CProver.nondetLong()";
      else if(java_type_string == "float")
        std::cout << "CProver.nondetFloat()";
      else if(java_type_string == "double")
        std::cout << "CProver.nondetDouble()";
      else
        std::cout << "CProver.nondetWithNull<" << java_type_string << ">()";
      std::cout << ";\n";
    }
    else
    {
      exprt def = id.second.definition;
      expand_function_applications(def);
      std::cout << "    " << type2java(type, ns) << " " << identifier << " = "
                << expr2java(def, ns) << ";\n";
    }

    constants_done.insert(identifier);
  }
}

void smt2tojavat::expand_function_applications(exprt &expr)
{
  for(exprt &op : expr.operands())
    expand_function_applications(op);

  if(expr.id() == ID_function_application)
  {
    auto &app = to_function_application_expr(expr);

    // look it up
    irep_idt identifier = app.function().get_identifier();
    auto f_it = id_map.find(identifier);

    if(f_it != id_map.end())
    {
      const auto &f = f_it->second;

      DATA_INVARIANT(
        f.type.id() == ID_mathematical_function,
        "type of function symbol must be mathematical_function_type");

      const auto f_type = to_mathematical_function_type(f.type);

      const auto &domain = f_type.domain();

      DATA_INVARIANT(
        domain.size() == app.arguments().size(),
        "number of function parameters");

      replace_symbolt replace_symbol;

      std::map<irep_idt, exprt> parameter_map;
      for(std::size_t i = 0; i < domain.size(); i++)
      {
        const symbol_exprt s(f.parameters[i], domain[i]);
        replace_symbol.insert(s, app.arguments()[i]);
      }

      exprt body = f.definition;
      replace_symbol(body);
      expand_function_applications(body);
      expr = body;
    }
  }
}

void smt2tojavat::command(const std::string &c)
{
  {
    if(c == "assert")
    {
      exprt e = expression();
      if(e.is_not_nil())
      {
        // add expression as constraint
        expand_function_applications(e);
        symbol_tablet symbol_table;
        namespacet ns(symbol_table);
        std::cout << "    CProver.assume(" << expr2java(e, ns) << ");\n";
      }
    }
    else if(c == "check-sat")
    {
      // ignore
    }
    else if(c == "check-sat-assuming")
    {
      throw error("not yet implemented");
    }
    else if(c == "display")
    {
      // ignore
    }
    else if(c == "get-value")
    {
      // ignore
    }
    else if(c == "echo")
    {
      // ignore
    }
    else if(c == "get-assignment")
    {
      // ignore
    }
    else if(c == "get-model")
    {
      // ignore
    }
    else if(c == "simplify")
    {
      // ignore
    }
    else
    {
      smt2_parsert::command(c);

      // add definitions
      define_constants();
    }
  }
}

class smt2_message_handlert : public message_handlert
{
public:
  void print(unsigned level, const std::string &message) override
  {
    message_handlert::print(level, message);

    if(level < 4) // errors
      std::cout << "(error \"" << message << "\")\n";
    else
      std::cout << "; " << message << '\n';
  }

  void print(unsigned, const xmlt &) override
  {
  }

  void print(unsigned, const jsont &) override
  {
  }

  void flush(unsigned) override
  {
    std::cout << std::flush;
  }
};

int smt2tojava(std::istream &in, const std::string classname)
{
  symbol_tablet symbol_table;
  namespacet ns(symbol_table);

  smt2_message_handlert message_handler;
  messaget message(message_handler);

  // this is our default verbosity
  message_handler.set_verbosity(messaget::M_STATISTICS);

  smt2tojavat smt2tojava(in);
  bool error_found = false;

  std::cout << "import org.cprover.CProver;\n\n";
  std::cout << "public class " << classname << "{\n";
  std::cout << "  public static void main(String[] args) {\n";

  while(!smt2tojava.exit)
  {
    try
    {
      smt2tojava.parse();
    }
    catch(const smt2_tokenizert::smt2_errort &error)
    {
      smt2tojava.skip_to_end_of_list();
      error_found = true;
      message.error().source_location.set_line(error.get_line_no());
      message.error() << error.what() << messaget::eom;
    }
    catch(const analysis_exceptiont &error)
    {
      smt2tojava.skip_to_end_of_list();
      error_found = true;
      message.error() << error.what() << messaget::eom;
    }
  }

  std::cout << "    assert(false);\n";
  std::cout << "  }\n";
  std::cout << "}\n";

  if(error_found)
    return 20;
  else
    return 0;
}

int main(int argc, const char *argv[])
{
  if(argc == 1)
    return smt2tojava(std::cin, "Smt2ToJava");

  if(argc != 2)
  {
    std::cerr << "usage: smt2tojava file\n";
    return 1;
  }

  std::ifstream in(argv[1]);
  if(!in)
  {
    std::cerr << "failed to open " << argv[1] << '\n';
    return 1;
  }

  std::string classname(argv[1]);
  classname.erase(classname.find_last_of("."), std::string::npos);
  classname.erase(0, classname.find_last_of("/") + 1);
  while(auto pos = classname.find('-') != std::string::npos)
    classname.replace(pos, pos + 1, "_");

  return smt2tojava(in, classname);
}
