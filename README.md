# unrealPrimitiveMesh

目前是在unreal 5.4上做的，参照的这个链接[link](https://www.zhihu.com/people/zhang-yu-hao-38-26/posts)

# 0
如果要在renderdoc里调试unreal的shader的话，在ue5的ConsoleVariable.ini里面要加上下面的几个：

```
r.Shaders.Optimize=0
r.Shaders.KeepDebugInfo=1
r.Shaders.Symbols=1
r.Shaders.ExtraData=1
```
之后再在renderdoc里截帧就可以得到对应的mesh在shader里的计算shader了

# 1
vertex factory 顶点工厂基础[link](https://zhuanlan.zhihu.com/p/695825915)

# 2
vertex factory manual fetch [link](https://zhuanlan.zhihu.com/p/695826868)

这里要注意的点就是对于实际的shader parameter struct的实现，要放在cpp文件里也就是这句话要在cpp里实现：
```IMPLEMENT_GLOBAL_SHADER_PARAMETER_STRUCT(FCustomVertexFactoryParameters, "CustomVF");```