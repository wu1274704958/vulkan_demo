#include <comm_comp/scene.hpp>
#include <comm_comp/showcase.hpp>

namespace vkd
{
	std::weak_ptr<Scene> Scene::get_scene()
	{
		return std::dynamic_pointer_cast<Scene>(shared_from_this());
	}

	std::shared_ptr<const Camera> Scene::get_camera() const
	{
		for(int64_t i = static_cast<int64_t>(cameras.size()) - 1;i >= 0;--i)
		{
			if(const auto& p = cameras[i].lock();p && p->is_enable())
			{
				return p;
			}
		}
		return nullptr;
	}

	void Scene::add_camera(std::weak_ptr<Camera> cam)
	{
		//if(!has_camera(cam))
		{
			cameras.push_back(cam);
		}
	}

	bool Scene::has_camera(std::weak_ptr<Camera> cam) const
	{
		return false;
	}

	bool Scene::rm_camera(std::shared_ptr<Camera> cam)
	{
		int64_t k = -1;
		for (int64_t i = static_cast<int64_t>(cameras.size()) - 1; i >= 0; --i)
		{
			if (const auto& p = cameras[i].lock(); p && p == cam)
			{
				k = i;break;
			}
		}
		if(k >= 0)
		{
			 cameras.erase(cameras.begin() + k);
			 return true;
		}
		return false;
	}




}
