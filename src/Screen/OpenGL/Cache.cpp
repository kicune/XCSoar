/*
Copyright_License {

  XCSoar Glide Computer - http://www.xcsoar.org/
  Copyright (C) 2000-2011 The XCSoar Project
  A detailed list of copyright holders can be found in the file "AUTHORS".

  This program is free software; you can redistribute it and/or
  modify it under the terms of the GNU General Public License
  as published by the Free Software Foundation; either version 2
  of the License, or (at your option) any later version.

  This program is distributed in the hope that it will be useful,
  but WITHOUT ANY WARRANTY; without even the implied warranty of
  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
  GNU General Public License for more details.

  You should have received a copy of the GNU General Public License
  along with this program; if not, write to the Free Software
  Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.
}
*/

#include "Screen/OpenGL/Cache.hpp"
#include "Screen/OpenGL/Texture.hpp"
#include "Screen/Color.hpp"

#include <map>
#include <string>
#include <assert.h>

struct ListHead {
  ListHead *prev, *next;
};

struct RenderedText : public ListHead {
  std::string key;
  GLTexture texture;

#ifdef ANDROID
  RenderedText(const char *_key, int id, unsigned width, unsigned height)
    :key(_key), texture(id, width, height) {}
#else
  RenderedText(const char *_key, SDL_Surface *surface)
    :key(_key), texture(surface) {}
#endif
};

static std::map<std::string,RenderedText*> text_cache_map;
static ListHead text_cache_head = { &text_cache_head, &text_cache_head };
static unsigned text_cache_size = 0;

#ifdef ANDROID
GLTexture *
TextCache::get(const Font *font, Color background_color, Color text_color,
               const char *text)
#else
GLTexture *
TextCache::get(TTF_Font *font, Color background_color, Color text_color,
               const char *text)
#endif
{
  assert(font != NULL);
  assert(text != NULL);

  char key[4096];
  snprintf(key, sizeof(key),
#ifdef ANDROID
           "%s_%u_%u_%02x%02x%02x_%02x%02x%02x_%s",
           font->get_facename(),
           font->get_style(),
           font->get_height(),
#else  // !ANDROID
           "%s_%s_%u_%02x%02x%02x_%02x%02x%02x_%s",
           TTF_FontFaceFamilyName(font),
           TTF_FontFaceStyleName(font),
           TTF_FontHeight(font),
#endif
           background_color.red(),
           background_color.green(),
           background_color.blue(),
           text_color.red(),
           text_color.green(),
           text_color.blue(),
           text);

  /* look it up */

  std::map<std::string,RenderedText*>::const_iterator i =
    text_cache_map.find(key);
  if (i != text_cache_map.end()) {
    /* found in the cache */
    RenderedText *rt = i->second;

    /* move to the front, so it gets flushed last */
    if (rt->prev != &text_cache_head) {
      /* remove */
      rt->next->prev = rt->prev;
      rt->prev->next = rt->next;

      /* insert */
      rt->next = text_cache_head.next;
      rt->next->prev = rt;
      rt->prev = &text_cache_head;
      text_cache_head.next = rt;
    }

    return &rt->texture;
  }

  /* remove old entries from cache */

  if (text_cache_size >= 256) {
    RenderedText *rt = (RenderedText *)text_cache_head.prev;
    text_cache_map.erase(rt->key);
    rt->next->prev = rt->prev;
    rt->prev->next = rt->next;
    delete rt;
    --text_cache_size;
  }

  /* render the text into a OpenGL texture */

#ifdef ANDROID
  SIZE size;
  int texture_id = font->text_texture_gl(text, size, text_color, background_color);
  RenderedText *rt = new RenderedText(key, texture_id, size.cx, size.cy);
#else
  SDL_Surface *surface = ::TTF_RenderUTF8_Solid(font, text, Color::BLACK);
  if (surface == NULL)
    return NULL;

  surface->flags &= ~SDL_SRCCOLORKEY;
  if (surface->format->palette != NULL &&
      surface->format->palette->ncolors >= 2) {
    surface->format->palette->colors[0] = background_color;
    surface->format->palette->colors[1] = text_color;
  }

  /* insert into cache */

  RenderedText *rt = new RenderedText(key, surface);
  SDL_FreeSurface(surface);
#endif

  rt->next = text_cache_head.next;
  rt->next->prev = rt;
  rt->prev = &text_cache_head;
  text_cache_head.next = rt;

  text_cache_map.insert(std::make_pair(key, rt));

  ++text_cache_size;

  /* done */

  return &rt->texture;
}

void
TextCache::flush()
{
  text_cache_map.clear();

  for (RenderedText *rt = (RenderedText *)text_cache_head.next;
       rt != &text_cache_head;) {
    RenderedText *next = (RenderedText *)rt->next;
    delete rt;
    rt = next;
  }

  text_cache_head.prev = text_cache_head.next = &text_cache_head;
}
