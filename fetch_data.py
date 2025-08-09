import requests
import pandas as pd
from io import StringIO

years = [1980, 1982, 1984, 1986, 1988, 1991, 1995, 1999, 2002, 2007]
year_data_cache = {}

def get_url(year):
    base_num = 39 - (2007 - year)
    if year == 1980:
        return "https://www.mca.gov.cn/mzsj/tjbz/a/201713/201708040959.html"
    if year == 2002:
        return "https://www.mca.gov.cn/mzsj/tjbz/a/201713/201708220927.html"
    if year == 1999:
        return "https://www.mca.gov.cn/mzsj/tjbz/a/201713/201708220921.html"
    if year == 1988:
        return "https://www.mca.gov.cn/mzsj/tjbz/a/201713/201708220903.html"
    else:
        return f"https://www.mca.gov.cn/mzsj/tjbz/a/201713/2017082209{base_num:02d}.html"

def fetch_table(url):
    try:
        resp = requests.get(url, timeout=10)
        resp.encoding = resp.apparent_encoding
        if resp.status_code == 200 and "<table" in resp.text:
            dfs = pd.read_html(StringIO(resp.text))
            if dfs:
                df = dfs[0]

                keywords = ["1.", "行政区划代码", "行政区划名称", "中华人民共和国行政区划代码"]
                def is_header_row(row):
                    row_text = " ".join(str(x) for x in row.values)
                    for kw in keywords:
                        if kw in row_text:
                            return True
                    return False
                df = df[~df.apply(is_header_row, axis=1)].reset_index(drop=True)

                keywords_comment = ["注：", "行政区划变更依据", "行政区划代码不含"]
                def is_comment_row(row):
                    row_text = " ".join(str(x) for x in row.values)
                    for kw in keywords_comment:
                        if kw in row_text:
                            return True
                    return False
                df = df[~df.apply(is_comment_row, axis=1)]

                df = df.fillna('')

                return df
    except Exception:
        pass
    return None

def load_year_data(year):
    if year in year_data_cache:
        return True
    url = get_url(year)
    df = fetch_table(url)
    if df is not None and not df.empty:
        year_data_cache[year] = df
        return True
    else:
        year_data_cache[year] = None
        return False

def query_admin_name(code: str, birth_year: int) -> str:
    # 找出小于等于birth_year的最大年份
    valid_years = [y for y in years if y <= birth_year]
    if not valid_years:
        return ''
    query_year = max(valid_years)

    if query_year not in year_data_cache or year_data_cache[query_year] is None:
        if not load_year_data(query_year):
            return ''

    df = year_data_cache.get(query_year)
    if df is None:
        return ''

    # 校验code长度
    if len(code) != 6:
        return ''

    # 提取省、市、区代码
    province_code = code[:2] + "0000"
    city_code = code[:4] + "00"
    district_code = code

    names = []

    for c in [province_code, city_code, district_code]:
        filtered_rows = df[df.iloc[:, 1] == c]
        if not filtered_rows.empty:
            name = str(filtered_rows.iloc[0, 2]).strip()
            if name and name not in names:
                names.append(name)

    return ''.join(names)
