#ifndef __PYTHON_OPTIONAL__
#define __PYTHON_OPTIONAL__

#include <boost/python.hpp>
#include <boost/optional.hpp>

template <typename T, typename TfromPy>
struct object_from_python
{
	object_from_python()
	{
		boost::python::converter::registry::push_back
		(&TfromPy::convertible, &TfromPy::construct,
		 boost::python::type_id<T>());
	}
};

template <typename T, typename TtoPy, typename TfromPy>
struct register_python_conversion
{
	register_python_conversion()
	{
		boost::python::to_python_converter<T, TtoPy>();
		object_from_python<T, TfromPy>();
	}
};


template <typename T>
struct register_optional : public boost::noncopyable
{
	struct optional_to_python
	{
		static PyObject* convert(const boost::optional<T>& value)
		{
			return (value ? boost::python::to_python_value<T>()(*value) :
			        boost::python::detail::none());
		}
	};

	struct optional_from_python
	{
		static void* convertible(PyObject* source)
		{
			using namespace boost::python::converter;

			if(source == Py_None)
				return source;

			const registration& converters(registered<T>::converters);

			if(implicit_rvalue_convertible_from_python(source,
			    converters))
			{
				rvalue_from_python_stage1_data data =
				  rvalue_from_python_stage1(source, converters);
				return rvalue_from_python_stage2(source, data, converters);
			}
			return NULL;
		}

		static void construct(PyObject* source,
		                      boost::python::converter::rvalue_from_python_stage1_data* data)
		{
			using namespace boost::python::converter;

			void* const storage = ((rvalue_from_python_storage<boost::optional<T> >*)
			                       data)->storage.bytes;

			if(data->convertible == source)             // == None
				new(storage) boost::optional<T>();  // A Boost uninitialized value
			else
				new(storage) boost::optional<T>(*static_cast<T*>(data->convertible));

			data->convertible = storage;
		}
	};

	explicit register_optional()
	{
		register_python_conversion < boost::optional<T>,
		                           optional_to_python, optional_from_python > ();
	}
};

template <>
struct register_optional<double> : public boost::noncopyable
{
	struct optional_to_python
	{
		static PyObject* convert(const boost::optional<double>& value)
		{
			return (value ? PyFloat_FromDouble(*value) :
			        boost::python::detail::none());
		}
	};

	struct optional_from_python
	{
		static void* convertible(PyObject* source)
		{
			using namespace boost::python::converter;

			if(source == Py_None || PyFloat_Check(source))
				return source;

			return 0;
		}

		static void construct(PyObject* source,
		                      boost::python::converter::rvalue_from_python_stage1_data* data)
		{
			assert(data->convertible == source);

			using namespace boost::python::converter;

			void* const storage = ((rvalue_from_python_storage<boost::optional<double> >*)
			                       data)->storage.bytes;

			if(source == Py_None)         // == None
				new(storage) boost::optional<double>();  // A Boost uninitialized value
			else
				new(storage) boost::optional<double>(PyFloat_AsDouble(source));

			data->convertible = storage;
		}
	};

	explicit register_optional()
	{
		register_python_conversion < boost::optional<double>,
		                           optional_to_python, optional_from_python > ();
	}
};



#endif //__PYTHON_OPTIONAL__