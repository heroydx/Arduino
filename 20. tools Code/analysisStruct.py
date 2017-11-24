# coding=utf-8
import struct
import re

# １:数据成员对齐规则：结构(struct)(或联合(union))
# 的数据成员，第一个数据成员放在offset为0的地方，以后每个数据成员存储的起始位置要从该成员大小或者成员的子成员大小（只要该成员有子成员，比如说是数组，结构体等）的整数倍开始(比如int在３２位机为４字节, 则要从４的整数倍地址开始存储。
#
#
#
# ２:结构体作为成员:如果一个结构里有某些结构体成员, 则结构体成员要从其内部最大元素大小的整数倍地址开始存储.(struct
# a里存有struct
# b, b里有char, int, double等元素, 那b应该从8的整数倍开始存储.)
#
#
#
# ３:收尾工作:结构体的总大小, 也就是sizeof的结果,.必须是其内部最大成员的整数倍.不足的要补齐.


mapFormat = {}
mapSize = {}

mapStructVal = {}  #key:name val:StructVal


SYSTEMSIZE = 4

mapFormat['char'] = 'c'
mapFormat['signed char'] =  'b'
mapFormat['unsigned char'] = 'B'
mapFormat['uchar'] = 'B'
mapFormat['bool'] = 'B'
mapFormat['short'] = 'h'
mapFormat['unsigned short'] = 'H'
mapFormat['ushort'] = 'H'
mapFormat['unsigned int'] = 'I'
mapFormat['uint'] = 'I'
mapFormat['int'] = 'i'
mapFormat['long'] = 'l'
mapFormat['unsigned long'] = 'L'
mapFormat['ulong'] = 'L'
mapFormat['long long'] = 'q'
mapFormat['unsigned long long'] = 'Q'
mapFormat['float'] = 'f'
mapFormat['double'] = 'd'
mapFormat['uchar*'] = 'p'


mapSize['char'] = 1
mapSize['signed char'] =  1
mapSize['unsigned char'] = 1
mapSize['uchar'] = 1
mapSize['bool'] = 1
mapSize['short'] = 2
mapSize['unsigned short'] = 2
mapSize['ushort'] = 2
mapSize['unsigned int'] = 4
mapSize['uint'] = 4
mapSize['int'] = 4
mapSize['long'] = 4
mapSize['unsigned long'] = 4
mapSize['ulong'] = 4
mapSize['long long'] = 8
mapSize['unsigned long long'] = 8
mapSize['float'] = 4
mapSize['double'] = 8
mapSize['uchar*'] = SYSTEMSIZE
# 构建class 用于存储结构的name 以及 val,用于后面的排序
class StructVal(object):
    def __init__(self):
        self.name = ''
        self.vals = []
        self.format = ''
        self.maxValSize = 0
        self.totalSize = 0


##用于filter list里的空字符串
def delNUll(x):
    return x != ''

##解析文本函数返回 一个 StructVal
def funAnalyzeStruct(name,content):
    lines = content.splitlines()
    structVal =  StructVal()
    structVal.name = name.lower().strip()
    for line in lines:
        line = line.strip()
        sublistructValnes = re.split('\s',line)
        sublines = filter(delNUll,sublines)
        if sublines != []:
            tempResult = []
            tempResult.append(sublines[0].lower())
            tempResult.append(sublines[1].lower())
            structVal.vals.append(tempResult)
    return


def comp(x,y):
    for val in x.vals:
        if mapFormat.has_key(val[0].lower()) == False:
            return 1
    for val in y.vals:
        if mapFormat.has_key(val[0].lower()) == False:
            return -1
    return 0

if __name__ == '__main__':
    with open('/Users/huxuesong/Downloads/Link16_Signal/GlobalSaveVariable.h','r') as stuFile:
        content = stuFile.read().decode("gb2312")
        pattern = re.compile('typedef struct (.*?){(.*?)}(.*?);',re.S)
        findNumberPattern = re.compile('\[(.*?)\];',re.S)
        items = re.findall(pattern, content)
        results = []
        for item in items:
            result = funAnalyzeStruct(item[2],item[1])
            results.append(result)
        results.sort(comp)
        # 开始计算每一个format和size
        for i in range(0,len(results)):
            structVal = results[i]
            totalSize = 0
            totalFormat = ''
            for val in structVal.vals:
                # 1 如果是基础类型
                if mapFormat.has_key(val[0]) == True:
                    curSize = mapSize[val[0]]
                    structVal.maxValSize = max(structVal.maxValSize, mapSize[val[0]])
                    numbers = re.findall(findNumberPattern,val[1])
                    numVal = 1
                    if len(numbers) > 0 and numbers[0].isalnum():
                        numVal = int(numbers[0])
                    if totalSize % curSize != 0:
                        remainSize = (curSize - totalSize%curSize)
                        totalSize += remainSize
                        #totalFormat += 'B' * remainSize
                    if numVal > 1:
                        totalFormat = totalFormat + str(numVal)+mapFormat[val[0]]
                    else:
                        totalFormat = totalFormat + mapFormat[val[0]]
                    totalSize = totalSize + numVal* mapSize[val[0]]

                 # 2 如果是自定义结构体类型
                else:
                    childStru = mapStructVal[val[0]]
                    structVal.maxValSize = max(childStru.maxValSize,structVal.maxValSize)
                    curSize = childStru.totalSize
                    maxValSize = childStru.maxValSize
                    if totalSize % maxValSize != 0:
                        remainSize = (maxValSize - totalSize%maxValSize)
                        totalSize += remainSize
                        totalFormat += 'x' * remainSize
                    totalFormat = totalFormat + childStru.format
                    totalSize = totalSize + childStru.totalSize
            if totalSize % structVal.maxValSize != 0:
                remainSize = (structVal.maxValSize - totalSize % structVal.maxValSize)
                totalSize += remainSize
                totalFormat += 'x' * remainSize
            structVal.totalSize = totalSize
            structVal.format = totalFormat
            mapStructVal[structVal.name] = structVal



        for result in results:
            print result.name +" "+result.format+" "+str(result.totalSize) + " "+str(struct.calcsize(result.format))

