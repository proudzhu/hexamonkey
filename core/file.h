#ifndef FILE_H
#define FILE_H

#include <iostream>
#include <fstream>
#include <stdint.h>


/** \brief High-level input stream operations on files with bit precision

The class is implemented as an adaptor for a std::ifstream instance that
reimplements common operation with bit precision instead of byte precision*/
class File
{
public:
    File();

    /** \brief Sets the path to the file*/
    void setPath(const std::string& path);

    /** \brief Returns the path to the file*/
    const std::string& path() const;

    /** \brief Opens stream with the given path*/
    void open();

    /** \brief Closes stream*/
    void close();

    /** \brief Clears the file error flags*/
    void clear();


    /** \brief Extracts bits from stream

    Puts the result in a byte array already allocated
    the result is right aligned and zero padded*/
    void read(char* s, int64_t size);

    /** \brief Offsets the position

     * \param off Offset to apply in bits.
     * \param dir Where to start from to apply the offset.
     * begin (std::ios_base::beg), current (std::ios_base::cur) or
     * end (std::ios_base::end).
     */
    void seekg(int64_t off, std::ios_base::seekdir dir);

    /** \brief Returns the current stream position */
    int64_t tellg();

    /** \brief Returns the size of the file*/
    int64_t size();

    /** \brief Checks if data can be recovered from the stream*/
    bool good();


private:
    File& operator=(const File&) = delete;
    File(const File&) = delete;

    std::string _path;
    std::ifstream _file;

    char _bitPosition;
};

#endif // FILE_H
