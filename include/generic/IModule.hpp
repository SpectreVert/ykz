/*
 * Created by Costa Bushnaq
 *
 * 04-05-2021 @ 19:54:38
 *
 * see LICENSE
*/

#ifndef _IMODULE_HPP
#define _IMODULE_HPP

namespace ykz {

struct IModule {
	virtual ~IModule() = default;

	virtual void zonze() = 0;
};

} // namespace ykz

#endif /* _IMODULE_HPP */
