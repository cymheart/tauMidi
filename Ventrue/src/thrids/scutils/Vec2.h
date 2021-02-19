#ifndef _Vec2_h_
#define _Vec2_h_
#include"Utils.h"

namespace scutils
{
    struct Vec2
    {   
    public:
        double x; 
        double y;
    public:
        Vec2(double p_x = 0, double p_y = 0) 
        {
            x = p_x;
            y = p_y;
        }

        double sqrMagnitude()
        {
            return x * x + y * y;
        }
        
        double magnitude()
        {
            return sqrt(sqrMagnitude());
        }

      
        Vec2 normalized()
        {
            Vec2 result(x, y);
            result.Normalize();
            return result;
        }
                                
        // 单位化
        void Normalize()
        {
            if (x != 0 || y!= 0)
            {
                double length = magnitude();
                x /= length;
                y /= length;
            }
        }

        // 角度
        static float Angle(Vec2 from, Vec2 to)
        {
            double cos = Dot(from.normalized(), to.normalized());
            if (cos < -1)
            {
                cos = -1;
            }
            if (cos > 1)
            {
                cos = 1;
            }
            return (float)(acos(cos) * (180 / M_PI));
        }

  
        // 限制距离
                                               
        static Vec2 ClampMagnitude(Vec2 vec, double maxLength) 
        {
            if (maxLength * maxLength >= vec.sqrMagnitude())
            {
                return vec;
            }
            return  maxLength * vec.normalized();
        }
          
        Vec2 operator+(Vec2 b)
        {
            return Vec2(x + b.x, y + b.y);
        }

        Vec2 operator-(Vec2 b)
        {
            return Vec2(x - b.x, y - b.y);
        }

        friend Vec2 operator-(Vec2 vec)
        {
            return Vec2(-vec.x, -vec.y);
        }

        Vec2 operator*(double b)
        {
            return Vec2(x * b, y * b);
        }

        friend Vec2 operator*(double a, Vec2 vec)
        {
            return Vec2(a * vec.x, a * vec.y);
        }

        Vec2 operator/( double b)
        {
            return Vec2(x / b, y / b);
        }

        bool operator ==(Vec2 rhs)
        {
            if (x == rhs.x && y == rhs.y)
            {
                return true;
            }
            else
            {
                return false;
            }
        }

        bool operator !=(Vec2 rhs)
        {
            if (x == rhs.x && y == rhs.y)
            {
                return false;
            }
            else
            {
                return true;
            }
        }

        //距离
        static double Distance(Vec2 a, Vec2 b) 
        {
            return sqrt((a.x - b.x) * (a.x - b.x) + (a.y - b.y) * (a.y - b.y));
        }

        //点乘
        static double Dot(Vec2 lhs, Vec2 rhs)
        {
            return lhs.x * rhs.x + lhs.y * rhs.y;
        }

        // 线性插值      
        static Vec2 Lerp(Vec2 a, Vec2 b, double t)
        {
            if (t <= 0)
            {
                return a;
            }
            else if (t >= 1)
            {
                return b;
            }
            return a + (b - a) * t;
        }

        // 线性插值(无限制)
        static Vec2 LerpUnclamped(Vec2 a, Vec2 b, double t)
        {
            return a + (b - a) * t;
        }
         
        // 最大值(X,Y均取最大)
        static Vec2 Max(Vec2 lhs, Vec2 rhs) 
        {
            Vec2 temp(0,0);
            temp.x = max(lhs.x, rhs.x);
            temp.y = max(lhs.y, rhs.y);
            return temp;
        }
         
        // 最小值(X,Y均取最小)
        static Vec2 Min(Vec2 lhs, Vec2 rhs)
        {
            Vec2 temp(0, 0);
            temp.x = min(lhs.x, rhs.x);
            temp.y = min(lhs.y, rhs.y);
            return temp;
        }
     
        // 缩放
        static Vec2 Scale(Vec2 a, Vec2 b)
        {
            Vec2 temp(0,0);
            temp.x = a.x * b.x;
            temp.y = a.y * b.y;
            return temp;
        }

        // 模长平方
        static double SqrMagnitude(Vec2 a)
        {
            return a.sqrMagnitude();
        }

        // 缩放
        void Scale(Vec2 scale)
        {
            x *= scale.x;
            y *= scale.y;
        }

        // 设置向量
        void Set(double newX, double newY)
        {
            x = newX;
            y = newY;
        }                                                                                           
    };
}

#endif