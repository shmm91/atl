#include "atl/atl_test.hpp"
#include "atl/utils/data.hpp"
#include "atl/utils/config.hpp"

#define TEST_CONFIG "tests/configs/config/config.yaml"


TEST(Utils_config_ConfigParam, constructor) {
  atl::ConfigParam param;

  ASSERT_EQ(atl::TYPE_NOT_SET, param.type);
  ASSERT_EQ("", param.key);
  ASSERT_EQ(false, param.optional);

  ASSERT_EQ(NULL, param.b);
  ASSERT_EQ(NULL, param.i);
  ASSERT_EQ(NULL, param.f);
  ASSERT_EQ(NULL, param.d);
  ASSERT_EQ(NULL, param.s);

  ASSERT_EQ(NULL, param.b_array);
  ASSERT_EQ(NULL, param.i_array);
  ASSERT_EQ(NULL, param.f_array);
  ASSERT_EQ(NULL, param.d_array);
  ASSERT_EQ(NULL, param.s_array);

  ASSERT_EQ(NULL, param.vec2);
  ASSERT_EQ(NULL, param.vec3);
  ASSERT_EQ(NULL, param.vec4);
  ASSERT_EQ(NULL, param.vecx);

  ASSERT_EQ(NULL, param.mat2);
  ASSERT_EQ(NULL, param.mat3);
  ASSERT_EQ(NULL, param.mat4);
  ASSERT_EQ(NULL, param.matx);
}

TEST(Utils_config_ConfigParser, constructor) {
  atl::ConfigParser parser;

  ASSERT_FALSE(parser.configured);
  ASSERT_FALSE(parser.loaded);
}

TEST(Utils_config_ConfigParser, addParam) {
  bool b;
  int i;
  float f;
  double d;
  std::string s;

  std::vector<bool> b_array;
  std::vector<int> i_array;
  std::vector<float> f_array;
  std::vector<double> d_array;
  std::vector<std::string> s_array;

  atl::Vec2 vec2;
  atl::Vec3 vec3;
  atl::Vec4 vec4;
  atl::VecX vecx;

  atl::Mat2 mat2;
  atl::Mat3 mat3;
  atl::Mat4 mat4;
  atl::MatX matx;
  cv::Mat cvmat;

  atl::ConfigParser parser;

  parser.addParam<bool>("bool", &b);
  parser.addParam<int>("int", &i);
  parser.addParam<float>("float", &f);
  parser.addParam<double>("double", &d);
  parser.addParam<std::string>("string", &s);

  parser.addParam<std::vector<bool>>("bool_array", &b_array);
  parser.addParam<std::vector<int>>("int_array", &i_array);
  parser.addParam<std::vector<float>>("float_array", &f_array);
  parser.addParam<std::vector<double>>("double_array", &d_array);
  parser.addParam<std::vector<std::string>>("string_array", &s_array);

  parser.addParam<atl::Vec2>("vector2", &vec2);
  parser.addParam<atl::Vec3>("vector3", &vec3);
  parser.addParam<atl::Vec4>("vector4", &vec4);
  parser.addParam<atl::VecX>("vector", &vecx);

  parser.addParam<atl::Mat2>("matrix2", &mat2);
  parser.addParam<atl::Mat3>("matrix3", &mat3);
  parser.addParam<atl::Mat4>("matrix4", &mat4);
  parser.addParam<atl::MatX>("matrix", &matx);
  parser.addParam<cv::Mat>("matrix", &cvmat);

  ASSERT_EQ(19, parser.params.size());
  ASSERT_EQ(atl::BOOL, parser.params[0].type);
  ASSERT_EQ("bool", parser.params[0].key);
  ASSERT_TRUE(parser.params[0].b != NULL);
}

TEST(Utils_config_ConfigParser, getYamlNode) {
  YAML::Node node1, node2;
  atl::ConfigParser parser;

  parser.load(TEST_CONFIG);

  parser.getYamlNode("level3.a.b.c", node1);
  ASSERT_EQ(3, node1.as<int>());

  parser.getYamlNode("float", node2);
  ASSERT_FLOAT_EQ(2.0, node2.as<float>());
}

TEST(Utils_config_ConfigParser, loadPrimitive) {
  int i;
  float f;
  double d;
  std::string s;
  atl::ConfigParser parser;
  atl::ConfigParam param;

  // setup
  parser.root = YAML::LoadFile(TEST_CONFIG);

  // INTEGER
  param.optional = false;
  param.type = atl::INT;
  param.key = "int";
  param.i = &i;
  parser.loadPrimitive(param);
  ASSERT_EQ(1, i);

  // FLOAT
  param.optional = false;
  param.type = atl::FLOAT;
  param.key = "float";
  param.f = &f;
  parser.loadPrimitive(param);
  ASSERT_FLOAT_EQ(2.0, f);

  // DOUBLE
  param.optional = false;
  param.type = atl::DOUBLE;
  param.key = "double";
  param.d = &d;
  parser.loadPrimitive(param);
  ASSERT_FLOAT_EQ(3.0, d);

  // STRING
  param.optional = false;
  param.type = atl::STRING;
  param.key = "string";
  param.s = &s;
  parser.loadPrimitive(param);
  ASSERT_EQ("hello world!", s);
}

TEST(Utils_config_ConfigParser, loadArray) {
  std::vector<bool> b_array;
  std::vector<int> i_array;
  std::vector<float> f_array;
  std::vector<double> d_array;
  std::vector<std::string> s_array;
  atl::ConfigParser parser;
  atl::ConfigParam param;

  // setup
  parser.root = YAML::LoadFile(TEST_CONFIG);

  // BOOL ARRAY
  param.optional = false;
  param.type = atl::BOOL_ARRAY;
  param.key = "bool_array";
  param.b_array = &b_array;
  parser.loadArray(param);

  ASSERT_TRUE(b_array[0]);
  ASSERT_FALSE(b_array[1]);
  ASSERT_TRUE(b_array[2]);
  ASSERT_FALSE(b_array[3]);

  // INTEGER
  param.optional = false;
  param.type = atl::INT_ARRAY;
  param.key = "int_array";
  param.i_array = &i_array;
  parser.loadArray(param);

  for (int i = 0; i < 4; i++) {
    ASSERT_EQ(i + 1, i_array[i]);
  }

  // FLOAT
  param.optional = false;
  param.type = atl::FLOAT_ARRAY;
  param.key = "float_array";
  param.f_array = &f_array;
  parser.loadArray(param);

  for (int i = 0; i < 4; i++) {
    ASSERT_FLOAT_EQ((float) i + 1.0, f_array[i]);
  }

  // DOUBLE
  param.optional = false;
  param.type = atl::DOUBLE_ARRAY;
  param.key = "double_array";
  param.d_array = &d_array;
  parser.loadArray(param);

  for (int i = 0; i < 4; i++) {
    ASSERT_FLOAT_EQ((double) i + 1.0, d_array[i]);
  }

  // STRING
  param.optional = false;
  param.type = atl::STRING_ARRAY;
  param.key = "string_array";
  param.s_array = &s_array;
  parser.loadArray(param);

  ASSERT_EQ("1.0", s_array[0]);
  ASSERT_EQ("2.0", s_array[1]);
  ASSERT_EQ("3.0", s_array[2]);
  ASSERT_EQ("4.0", s_array[3]);
}

TEST(Utils_config_ConfigParser, loadVector) {
  atl::Vec2 vec2;
  atl::Vec3 vec3;
  atl::Vec4 vec4;
  atl::VecX vecx;
  atl::ConfigParser parser;
  atl::ConfigParam param;

  // setup
  parser.root = YAML::LoadFile(TEST_CONFIG);

  // VECTOR 2
  param.optional = false;
  param.type = atl::VEC2;
  param.key = "vector2";
  param.vec2 = &vec2;
  parser.loadVector(param);

  ASSERT_FLOAT_EQ(1.0, vec2(0));
  ASSERT_FLOAT_EQ(2.0, vec2(1));

  // VECTOR 3
  param.optional = false;
  param.type = atl::VEC3;
  param.key = "vector3";
  param.vec3 = &vec3;
  parser.loadVector(param);

  ASSERT_FLOAT_EQ(1.0, vec3(0));
  ASSERT_FLOAT_EQ(2.0, vec3(1));
  ASSERT_FLOAT_EQ(3.0, vec3(2));

  // VECTOR 4
  param.optional = false;
  param.type = atl::VEC4;
  param.key = "vector4";
  param.vec4 = &vec4;
  parser.loadVector(param);

  ASSERT_FLOAT_EQ(1.0, vec4(0));
  ASSERT_FLOAT_EQ(2.0, vec4(1));
  ASSERT_FLOAT_EQ(3.0, vec4(2));
  ASSERT_FLOAT_EQ(4.0, vec4(3));

  // VECTOR X
  param.optional = false;
  param.type = atl::VECX;
  param.key = "vector";
  param.vecx = &vecx;
  parser.loadVector(param);

  for (int i = 0; i < 10; i++) {
    ASSERT_FLOAT_EQ((double) i + 1.0, vecx(i));
  }
}

TEST(Utils_config_ConfigParser, loadMatrix) {
  int index;
  atl::Mat2 mat2;
  atl::Mat3 mat3;
  atl::Mat4 mat4;
  atl::MatX matx;
  cv::Mat cvmat;
  atl::ConfigParser parser;
  atl::ConfigParam param;

  // setup
  parser.root = YAML::LoadFile(TEST_CONFIG);

  // MATRIX 2
  param.optional = false;
  param.type = atl::MAT2;
  param.key = "matrix2";
  param.mat2 = &mat2;
  parser.loadMatrix(param);

  ASSERT_FLOAT_EQ(1.0, mat2(0, 0));
  ASSERT_FLOAT_EQ(2.0, mat2(0, 1));
  ASSERT_FLOAT_EQ(3.0, mat2(1, 0));
  ASSERT_FLOAT_EQ(4.0, mat2(1, 1));

  // MATRIX 3
  param.optional = false;
  param.type = atl::MAT3;
  param.key = "matrix3";
  param.mat3 = &mat3;
  parser.loadMatrix(param);

  index = 0;
  for (int i = 0; i < 3; i++) {
    for (int j = 0; j < 3; j++) {
      ASSERT_FLOAT_EQ(index + 1.0, mat3(i, j));
      index++;
    }
  }

  // MATRIX 4
  param.optional = false;
  param.type = atl::MAT4;
  param.key = "matrix4";
  param.mat4 = &mat4;
  parser.loadMatrix(param);

  index = 0;
  for (int i = 0; i < 4; i++) {
    for (int j = 0; j < 4; j++) {
      ASSERT_FLOAT_EQ(index + 1.0, mat4(i, j));
      index++;
    }
  }

  // MATRIX X
  param.optional = false;
  param.type = atl::MATX;
  param.key = "matrix";
  param.matx = &matx;
  parser.loadMatrix(param);

  index = 0;
  for (int i = 0; i < 3; i++) {
    for (int j = 0; j < 4; j++) {
      ASSERT_FLOAT_EQ(index + 1.0, matx(i, j));
      index++;
    }
  }

  // CV MATRIX
  param.optional = false;
  param.type = atl::CVMAT;
  param.key = "matrix";
  param.cvmat = &cvmat;
  parser.loadMatrix(param);

  index = 0;
  for (int i = 0; i < 3; i++) {
    for (int j = 0; j < 4; j++) {
      ASSERT_FLOAT_EQ(index + 1.0, cvmat.at<double>(i, j));
      index++;
    }
  }
}

TEST(Utils_config_ConfigParser, load) {
  int retval;
  bool b;
  int i;
  float f;
  double d;
  std::string s;

  std::vector<bool> b_array;
  std::vector<int> i_array;
  std::vector<float> f_array;
  std::vector<double> d_array;
  std::vector<std::string> s_array;

  atl::Vec2 vec2;
  atl::Vec3 vec3;
  atl::Vec4 vec4;
  atl::VecX vecx;

  atl::Mat2 mat2;
  atl::Mat3 mat3;
  atl::Mat4 mat4;
  atl::MatX matx;
  cv::Mat cvmat;

  atl::ConfigParser parser;

  parser.addParam<bool>("bool", &b);
  parser.addParam<int>("int", &i);
  parser.addParam<float>("float", &f);
  parser.addParam<double>("double", &d);
  parser.addParam<std::string>("string", &s);

  parser.addParam<std::vector<bool>>("bool_array", &b_array);
  parser.addParam<std::vector<int>>("int_array", &i_array);
  parser.addParam<std::vector<float>>("float_array", &f_array);
  parser.addParam<std::vector<double>>("double_array", &d_array);
  parser.addParam<std::vector<std::string>>("string_array", &s_array);

  parser.addParam<atl::Vec2>("vector2", &vec2);
  parser.addParam<atl::Vec3>("vector3", &vec3);
  parser.addParam<atl::Vec4>("vector4", &vec4);
  parser.addParam<atl::VecX>("vector", &vecx);

  parser.addParam<atl::Mat2>("matrix2", &mat2);
  parser.addParam<atl::Mat3>("matrix3", &mat3);
  parser.addParam<atl::Mat4>("matrix4", &mat4);
  parser.addParam<atl::MatX>("matrix", &matx);
  parser.addParam<cv::Mat>("matrix", &cvmat);

  retval = parser.load(TEST_CONFIG);
  if (retval != 0) {
    FAIL();
  }

  std::cout << "bool: " << b << std::endl;
  std::cout << "int: " << i << std::endl;
  std::cout << "float: " << f << std::endl;
  std::cout << "double: " << d << std::endl;
  std::cout << "string: " << s << std::endl;
  std::cout << std::endl;

  std::cout << "vector2: " << vec2.transpose() << std::endl;
  std::cout << "vector3: " << vec3.transpose() << std::endl;
  std::cout << "vector4: " << vec4.transpose() << std::endl;
  std::cout << "vector: " << vecx.transpose() << std::endl;
  std::cout << std::endl;

  std::cout << "matrix2: \n" << mat2 << std::endl;
  std::cout << "matrix3: \n" << mat3 << std::endl;
  std::cout << "matrix4: \n" << mat4 << std::endl;
  std::cout << "matrix: \n" << matx << std::endl;
  std::cout << "cvmatrix: \n" << cvmat << std::endl;
  std::cout << std::endl;
}
