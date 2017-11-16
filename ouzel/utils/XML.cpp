// Copyright (C) 2017 Elviss Strazdins
// This file is part of the Ouzel engine.

#include <algorithm>
#include "XML.hpp"
#include "core/Engine.hpp"
#include "Log.hpp"
#include "Utils.hpp"

static const std::vector<uint32_t> commentStart = {0x3c, 0x21, 0x2d, 0x2d}; // <!--
static const std::vector<uint32_t> commentEnd = {0x2d, 0x2d, 0x3e}; // -->

static const std::vector<uint32_t> cDataStart = {0x3c, 0x21, 0x5b, 0x43, 0x44, 0x41, 0x54, 0x41, 0x5b}; // <![CDATA[
static const std::vector<uint32_t> cDataEnd = {0x5d, 0x5d, 0x3e}; // ]]>

static const std::vector<uint32_t> doctypeStart = {0x3c, 0x21}; // <!
static const std::vector<uint32_t> doctypeEnd = {0x3e}; // >
static const std::vector<uint32_t> doctypeListStart = {0x5b}; // [
static const std::vector<uint32_t> doctypeListEnd = {0x5d}; // ]

static const std::vector<uint32_t> processingInstructionStart = {0x3c, 0x3f}; // <?
static const std::vector<uint32_t> processingInstructionEnd = {0x3f, 0x3e}; // ?>

static const std::vector<uint32_t> tagStart = {0x3c}; // <
static const std::vector<uint32_t> tagEnd = {0x3e}; // >

namespace ouzel
{
    namespace xml
    {
        Node::Node()
        {
        }

        bool Node::parse(const std::vector<uint32_t>& utf32,
                         std::vector<uint32_t>::iterator& iterator)
        {
            if (iterator == utf32.end())
            {
                Log(Log::Level::ERR) << "Unexpected end of file";
                return false;
            }

            if (*iterator == '<')
            {
                ++iterator;
                
                if (iterator == utf32.end())
                {
                    Log(Log::Level::ERR) << "Unexpected end of file";
                    return false;
                }

                if (*iterator == '!') // <!
                {
                    ++iterator;
                    
                    if (iterator == utf32.end())
                    {
                        Log(Log::Level::ERR) << "Unexpected end of file";
                        return false;
                    }

                    if (*iterator == '-') // <!-
                    {
                        ++iterator;
                        
                        for (;;)
                        {
                            if (iterator == utf32.end())
                            {
                                Log(Log::Level::ERR) << "Unexpected end of file";
                                return false;
                            }

                            if (*iterator == '-')
                            {
                                if (iterator + 1 == utf32.end())
                                {
                                    Log(Log::Level::ERR) << "Unexpected end of file";
                                    return false;
                                }
                                
                                if (*(iterator + 1) == '-') // --
                                {
                                    if (iterator + 2 == utf32.end())
                                    {
                                        Log(Log::Level::ERR) << "Unexpected end of file";
                                        return false;
                                    }
                                    
                                    if (*(iterator + 2) == '>') // -->
                                    {
                                        break;
                                    }
                                    else
                                    {
                                        Log(Log::Level::ERR) << "Unexpected double-hyphen inside comment";
                                        return false;
                                    }
                                }
                            }

                            value += utf32to8(*iterator);
                            ++iterator;
                        }
                        
                        type = Type::COMMENT;
                    }
                    else if (*iterator == '[') // <![
                    {
                        return true;
                    }
                    else
                    {
                        return true;
                    }
                }
                else if (*iterator == '?') // <?
                {
                    // TODO: parse xml declaration <?xml ... ?>
                    return true;
                }
                else // <
                {
                    ++iterator;
                    
                    for (;;)
                    {
                        if (iterator == utf32.end())
                        {
                            Log(Log::Level::ERR) << "Unexpected end of file";
                            return false;
                        }

                        if (*iterator == ' ' || *iterator == '\r' || *iterator == '\n' || *iterator == '\t') // whitespace
                        {
                            break;
                        }
                        else
                        {
                            value += utf32to8(*iterator);
                        }
                    }
                    
                    return true;
                }
            }
            else
            {
                Log(Log::Level::ERR) << "Expected start of a tag";
                return false;
            }

            return true;
        }

        bool Node::encode(std::vector<uint8_t>& data) const
        {
            return true;
        }
        
        Data::Data()
        {
        }

        Data::Data(const std::string& filename, bool preserveComments, bool preserveDeclaration)
        {
            init(filename, preserveComments, preserveDeclaration);
        }

        Data::Data(const std::vector<uint8_t>& data, bool preserveComments, bool preserveDeclaration)
        {
            init(data, preserveComments, preserveDeclaration);
        }

        bool Data::init(const std::string& filename, bool preserveComments, bool preserveDeclaration)
        {
            std::vector<uint8_t> data;

            if (!engine->getFileSystem()->readFile(filename, data))
            {
                return false;
            }

            return init(data, preserveComments, preserveDeclaration);
        }

        bool Data::init(const std::vector<uint8_t>& data, bool preserveComments, bool preserveDeclaration)
        {
            std::vector<uint32_t> utf32;

            // BOM
            if (data.size() >= 3 &&
                data[0] == 0xEF &&
                data[1] == 0xBB &&
                data[2] == 0xBF)
            {
                bom = true;
                std::vector<uint8_t> newData(data.begin() + 3, data.end());
                utf32 = utf8to32(newData);
            }
            else
            {
                bom = false;
                utf32 = utf8to32(data);
            }

            std::vector<uint32_t>::iterator iterator = utf32.begin();

            while (iterator != utf32.end())
            {
                Node node;
                if (!node.parse(utf32, iterator))
                {
                    return false;
                }

                if ((preserveComments || node.getType() != Node::Type::COMMENT) &&
                    (preserveDeclaration || node.getType() != Node::Type::DECLARATION))
                {
                    children.push_back(node);
                }
            }

            // TODO: check if only single tag is in the children list

            return true;
        }
        
        bool Data::save(const std::string& filename) const
        {
            std::vector<uint8_t> data;
            
            if (!encode(data)) return false;
            
            return engine->getFileSystem()->writeFile(filename, data);
        }
        
        bool Data::encode(std::vector<uint8_t>& data) const
        {
            data.clear();
            
            for (const Node& node : children)
            {
                if (!node.encode(data)) return false;
            }

            return true;
        }
    } // namespace xml
}
