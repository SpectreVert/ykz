/*
 * Created by Costa Bushnaq
 *
 * 23-09-2021 @ 13:31:40
 *
 * see LICENSE
*/

#ifndef _MESSAGE_HPP
#define _MESSAGE_HPP

#include <string>

namespace ykz {

class Message {
	int m_stamp;
	std::string m_contents;

public:
	/* TODO: write other constructors */
	Message(int stamp, std::string contents);

}; // class Message

} // namespace ykz

#endif /* _MESSAGE_HPP */
