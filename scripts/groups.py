from typing import List


test = [1, 2, 3, 3, 1, 1, 3, 2]

schema = [
    (1, True),
    (2, False),
    (3, False)
]

desired_split = [
    [1, 2, 3],
    [3, 1],
    [1, 3, 2]
]



def split(cnt: int, groups: List[int], schema: List[List]):
    ans = []
    idx = 0
    for i in range(cnt):
        currgroup = []
        seen = set()
        if idx >= len(groups): raise ValueError("Bad group")
        for num in groups[idx:]:
            if num not in seen:
                currgroup.append(num)
                seen.add(num)
                idx+=1
            else:
                break


        ans.append(currgroup)
    return ans


print(split(4, test, schema))