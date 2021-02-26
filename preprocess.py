import pandas as pd

tmpmaterial = ''
tmpbatch = ''
idx = 0

def mark(srsobj):
    global tmpmaterial
    global tmpbatch
    global idx

    rtid = idx
    if srsobj['Material'] != tmpmaterial or srsobj['Batch'] != tmpbatch:
        
        tmpmaterial = srsobj['Material']
        tmpbatch = srsobj['Batch']
        idx += 1

        return int(rtid)
    else:
        return int(rtid - 1)
        

def unusefulnum(srsobj):
    tmp = srsobj['Reference'] #str
    if(tmp[0] >= 'A' and tmp[0] <= 'Z'):
        return True

#print(type(filterdtfobj == True)) pandas series

dtfobj1 = pd.read_csv('export.csv', sep = ',')
dtfobj1 = dtfobj1[['Material','Batch','Reference']].dropna()

filterdtfobj1 = dtfobj1[['Material','Batch', 'Reference']].duplicated(keep = 'first')
dtfobj1 = dtfobj1.loc[~filterdtfobj1]
filterdtfobj1 = dtfobj1.apply(unusefulnum, axis = 1)
dtfobj1 = dtfobj1.loc[filterdtfobj1 == True].reset_index(drop = True)
dtfobj1['idx'] = 0
dtfobj1['idx'] = dtfobj1.apply(mark, axis = 1)
dtfobj1['idx'] = dtfobj1['idx'].astype(int)
print(dtfobj1)
print('\n')

tmpdtf = dtfobj1[['Material','Batch','Reference']].copy(deep = True)
filterfortmp = tmpdtf[['Material','Batch']].duplicated(keep = 'first')
tmpdtf = tmpdtf.loc[~filterfortmp].reset_index(drop = True)

ckol = dtfobj1[['Reference','idx']]
print(ckol)
print('\n')

length = len(ckol)
print("number of CID is " + str(length))
print('\n')

maxidx = str(ckol.iloc[length - 1, 1])
ckol = ckol.sort_values(['Reference'], ascending = False).reset_index(drop = True)

with open('CheckOutList.txt', 'w') as fp:
    fp.write(str(length) + ' ' + maxidx + '\n')
    
    for i in range(0, length):
        tmp = ckol.iloc[i]
        for j, ss in enumerate(tmp):
            fp.write(str(ss))
            if(j < len(tmp) - 1):
                fp.write(' ')
        fp.write('\n')


dtfobj2 = pd.read_csv('input.csv', sep = ',')
filterdtfobj2 = dtfobj2[['Material','Batch']].duplicated(keep = 'first')
dtfobj2 = dtfobj2.loc[~filterdtfobj2].reset_index(drop = True).dropna()
dtfobj2['Material'] = dtfobj2['Material'].str.upper()
dtfobj2['Batch'] = dtfobj2['Batch'].str.upper()
print(dtfobj2)
print('\n')

compare = pd.merge(dtfobj2, tmpdtf, how = 'left', on = ['Material','Batch'])
compare = compare.loc[pd.isnull(compare['Reference'])]
compare = compare[['Material','Batch']]
print(compare)
print('\n')

with open('CIDmiss.txt', 'w') as fp:
    for i in range(0, len(compare)):
        tmp = compare.iloc[i]
        for j, ss in enumerate(tmp):
            fp.write(str(ss))
            if(j < len(tmp) - 1):
                fp.write(' ')
        fp.write('\n')
    fp.write('以上产品在系统中暂无CID记录！')

