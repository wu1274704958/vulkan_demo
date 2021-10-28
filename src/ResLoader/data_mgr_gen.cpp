#include <data_mgr.hpp>
#include <sundry.hpp>
#include <generator/Generator.hpp>

using namespace gld;

std::string GenSquareIndices<>::key_from_args()
{
	return "GenSquareIndices[0,1,2,0,2,3]";
}

GenSquareIndices<>::RealRetTy GenSquareIndices<>::load()
{
	return std::make_tuple(true, std::shared_ptr<std::vector<int>>(new std::vector<int>({ 0,1,2,0,2,3 })));
}

std::string GenSquareVertices<float,float>::key_from_args(float x,float y)
{
	return sundry::format_tup('#',x,y);
}

GenSquareVertices<float, float>::RealRetTy GenSquareVertices<float, float>::load(float x,float y)
{
	return std::make_tuple(true, std::shared_ptr<std::vector<float>>(new std::vector<float>(gen::quad(glm::vec2(x, y)))));
}