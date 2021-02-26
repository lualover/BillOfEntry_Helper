import pandas as pd


foundlist = []
with open('foundfiles.txt', 'r') as fp: # newline = '\n'
    read = fp.readline()
    while(read):
        foundlist.append(read.strip('\n'))
        read = fp.readline()

misslist = []
with open('missfiles.txt', 'r') as fp: # newline = '\n'
    fp.readline() #第一行是中文，故略过
    read = fp.readline()
    while(read):
        misslist.append(read.strip('\n'))
        read = fp.readline()

def unusefulnum(srsobj):
    tmp = srsobj['Reference'] #str
    if(tmp[0] >= 'A' and tmp[0] <= 'Z'):
        return True


dtfobj1 = pd.read_csv('export.csv', sep = ',')
dtfobj1 = dtfobj1[['Material','Batch','Reference']].dropna()

filterdtfobj1 = dtfobj1[['Material','Batch', 'Reference']].duplicated(keep = 'first')
dtfobj1 = dtfobj1.loc[~filterdtfobj1]
filterdtfobj1 = dtfobj1.apply(unusefulnum, axis = 1)
dtfobj1 = dtfobj1.loc[filterdtfobj1 == True].reset_index(drop = True)

l1 = [dtfobj1.loc[dtfobj1.Reference == f] for f in foundlist]
result1 = pd.DataFrame()
if(l1):
    result1 = pd.concat(l1)
    result1['found'] = "true"

l2 = [dtfobj1.loc[dtfobj1.Reference == f] for f in misslist]
result2 = pd.DataFrame()
if(l2):
    result2 = pd.concat(l2)
    result2['found'] = "false"

result = pd.concat([result1, result2])
filterdtfobj2 = result[['Material','Batch']].duplicated(keep = 'first') #SeriesObj
result = result.loc[~filterdtfobj2]

result.to_csv('result.csv', index = False, sep = ',')

