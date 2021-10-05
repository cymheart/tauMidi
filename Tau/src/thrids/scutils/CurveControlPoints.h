#ifndef _CurveControlPoints_h_
#define _CurveControlPoints_h_

#include"Utils.h"
#include "Vec2.h"

namespace scutils
{
	class CurveControlPoints
	{
	public:

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
		static void Compute(float xPoints[], float yPoints[], int len, float out_conPoints[], bool isClosedCurve = false);

	};
}

#endif
