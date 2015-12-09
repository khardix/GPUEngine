#include <geSG/AnimationManager.h>
#include <algorithm>

using namespace ge::sg;


void AnimationManager::addAnimation(std::shared_ptr<Animation>& animation)
{
   animations.push_back(animation);
}

void AnimationManager::removeAnimation(std::shared_ptr<Animation> animation)
{
   auto it = std::find(animations.begin(), animations.end(), animation);
   animations.erase(it);
   auto pit = std::find(playlist.begin(), playlist.end(), animation);
   playlist.erase(pit);
}

void AnimationManager::update(double t)
{
   std::for_each(playlist.begin(), playlist.end(), [t](std::shared_ptr<Animation> animation){animation->update(t); });
   removeFinishedAnimation();
}

void AnimationManager::pauseAnimation(std::shared_ptr<Animation> /*animation*/)
{
}

void AnimationManager::playAnimation(std::shared_ptr<Animation> animation)
{
   auto it = std::find(playlist.begin(), playlist.end(), animation);
   auto ait = std::find(animations.begin(), animations.end(), animation);
   if(it == playlist.end())
   {
      playlist.push_back(animation);
   }
   if(ait == animations.end())
   {
      animations.push_back(animation);
   }
}

void AnimationManager::removeFinishedAnimation()
{
   auto it = playlist.begin();
   for(; it != playlist.end(); )
   {
      if((*it)->getCurrentTime() >= (*it)->duration && (*it)->mode == Animation::Mode::ONCE)
      {
         auto rm = it;
         ++it;
         playlist.erase(rm);
      }
      else
      {
         ++it;
      }
   }
}