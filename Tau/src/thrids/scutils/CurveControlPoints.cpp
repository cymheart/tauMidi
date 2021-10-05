#include"CurveControlPoints.h"

namespace scutils
{
	/// <summary>
	/// 算法来自:AGG之贝塞尔插值
	/// 返回值:长度len*4的控制点位置数组[x1,y1,x2, y2], 按序每两个控制点对应一个曲线点
	/// 
	/// 1.首先，我们计算出多边形所有边线的中点，Ai。
	/// 
	/// 2.然后连接起相邻边中点，得到很多线段，记为 Ci 。并用图记的方法计算出 Bi 点。
	/// 
	/// 3.最后一步，只需要简单地将 Ci 进行平移，平移的路径就是每条线段上 Bi 到对应顶点的路径。
	/// 就这样，我们计算出了贝塞尔曲线的控制点，平滑的结果看起来也很棒。
	/// 
	/// 4.这里还可以做一点小小的改进，因为我们已经得到了一条决定控制点的直线，所以，我们可以根据需要，
	/// 使控制点在这条直线上移动，这样可以改变插值曲线的状态。我使用了一个与控制点和顶点初始距离相关的系数 K ，
	/// 用来沿直线移动控制点。控制点离顶点越远，图形看起来就越锐利。
	/// </summary>
	/// <param name="xPoints">xpos</param>
	/// <param name="yPoints">ypos</param>
	/// <param name="len">点个数</param>
	/// <param name="out_conPoints">返回:长度len*4的控制点位置数组[x1,y1,x2, y2], 按序每两个控制点对应一个曲线点</param>
	/// <param name="isClosedCurve">是否为闭合曲线，闭合曲线尾部点会连接头部点</param>
	void CurveControlPoints::Compute(float xPoints[], float yPoints[], int len, float out_conPoints[], bool isClosedCurve)
	{
		Vec2* p = new Vec2[len];


		for (int i = 0; i < len; i++)
		{
			p[i].Set(xPoints[i], yPoints[i]);
		}

		//
		Vec2 v1, v2, vleft, vright, va, vb, vc, vresult; //向量
		double n1, n2, nc;  //长度
		double s1, s2;  //单位向量缩放
		int conIdx = 0;  //控制点idx
		int start = 1;
		int end = len - 1;
		int nexti = 0;

		//
		if (!isClosedCurve)
		{
			v1 = p[1] - p[0];
			n1 = v1.magnitude();
			v1 = v1 / n1;
			va = p[0] + v1 * (n1 / 2);
			start = 1;
			end = len - 1;
			conIdx = 0;
			out_conPoints[conIdx++] = p[0].x;
			out_conPoints[conIdx++] = p[0].y;
			out_conPoints[conIdx++] = p[0].x;
			out_conPoints[conIdx++] = p[0].y;

			int conEndPos = end * 4;
			out_conPoints[conEndPos++] = p[len - 1].x;
			out_conPoints[conEndPos++] = p[len - 1].y;
			out_conPoints[conEndPos++] = p[len - 1].x;
			out_conPoints[conEndPos++] = p[len - 1].y;
		}
		else
		{
			v1 = p[0] - p[len - 1];
			n1 = v1.magnitude();
			v1 = v1 / n1;
			va = p[len - 1] + v1 * (n1 / 2);
			start = 0;
			end = len;
			conIdx = 0;
		}

		for (int i = start; i < end; i++)
		{
			if (i == len - 1) { nexti = 0; }
			else { nexti = i + 1; }

			//
			v2 = p[nexti] - p[i];      //计算点指向右边点的向量
			n2 = v2.magnitude();       //右边线段长度
			v2 = v2 / n2;              //v2单位向量
			vb = p[i] + v2 * (n2 / 2); //计算点指向右边线段中点向量

			//
			vc = vb - va;          //左线段中点指向右线段中点的向量
			nc = vc.magnitude();   //vc长度
			vc = vc / nc;          //vc单位化

			//
			s2 = n2 / (n1 + n2);   //右线段在左右两线段总长中的占比
			vright = vc * s2;      //vc方向上缩放s2长度为右控制点的向量

			//
			s1 = n1 / (n1 + n2);  //左线段在左右两线段总长中的占比
			vleft = -vc * s1;     //vc反方向上缩放s1长度为左控制点的向量

			//
			vresult = p[i] + vleft;
			out_conPoints[conIdx++] = vresult.x;
			out_conPoints[conIdx++] = vresult.y;
			vresult = p[i] + vright;
			out_conPoints[conIdx++] = vresult.x;
			out_conPoints[conIdx++] = vresult.y;


			//v2交换变为v1，作左边线段参数
			v1 = v2;
			n1 = n2;
			va = vb;
		}

		delete[] p;
	}
}
