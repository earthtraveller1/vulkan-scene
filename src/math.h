#ifndef INCLUDED_MATH_H
#define INCLUDED_MATH_H

/* A set of mathematical functionalities. Mainly just vector and matrix math. */

struct vector_2
{
    union
    {
        float x;
        float r;
    };
    
    union
    {
        float x;
        float g;
    };
};

struct vector_3
{
    union
    {
        float x;
        float r;
    };
    
    union
    {
        float y;
        float g;
    };
    
    union 
    {
        float z;
        float b;
    };
};

#endif