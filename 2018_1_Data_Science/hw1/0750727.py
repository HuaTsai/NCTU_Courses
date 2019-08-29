#!/usr/bin/env python3
import re
import sys
import time

import requests
from bs4 import BeautifulSoup
from colorama import Fore
from requests.adapters import HTTPAdapter
from requests.packages.urllib3.util.retry import Retry

PTT_URL = 'https://www.ptt.cc'
CRAWL_YEAR = 2017
START_URL = '/bbs/Beauty/index1990.html'
OMIT_URL = [
    'https://www.ptt.cc/bbs/Beauty/M.1490936972.A.60D.html',
    'https://www.ptt.cc/bbs/Beauty/M.1494776135.A.50A.html',
    'https://www.ptt.cc/bbs/Beauty/M.1503194519.A.F4C.html',
    'https://www.ptt.cc/bbs/Beauty/M.1504936945.A.313.html',
    'https://www.ptt.cc/bbs/Beauty/M.1505973115.A.732.html',
    'https://www.ptt.cc/bbs/Beauty/M.1507620395.A.27E.html',
    'https://www.ptt.cc/bbs/Beauty/M.1510829546.A.D83.html',
    'https://www.ptt.cc/bbs/Beauty/M.1512141143.A.D31.html']


def crawl():
    start_time = time.time()

    all_file = open('all_articles.txt', 'w')
    popular_file = open('all_popular.txt', 'w')
    r = soup = None
    finish = False
    while(not finish):
        if (r is None):
            r = get(PTT_URL + START_URL)
        else:
            r = get(next_page_url(soup))
        soup = BeautifulSoup(r.text, 'html.parser')
        rent_list = valid(soup.find_all(class_='r-ent'))
        y_start = year(rent_list[0])
        d_start = date(rent_list[0])
        y_end = year(rent_list[-1])
        d_end = date(rent_list[-1])
        print(Fore.YELLOW + 'Range of valid post: {}/{}/{}~{}/{}/{}'.format(
            y_start, int(d_start / 100), d_start % 100, y_end, int(d_end / 100), d_end % 100) + Fore.RESET)
        if (y_start == CRAWL_YEAR - 1 and y_end == CRAWL_YEAR - 1):
            print(Fore.YELLOW + 'No required data in this range' + Fore.RESET)
            rent_list = []
        elif (y_start == CRAWL_YEAR - 1 and y_end == CRAWL_YEAR):
            print(Fore.YELLOW + 'Find start of requirements, delete redundant data...' + Fore.RESET)
            rent_list = list(filter(lambda rent: year(rent) == CRAWL_YEAR, rent_list))
        elif (y_start == CRAWL_YEAR and y_end == CRAWL_YEAR + 1):
            print(Fore.YELLOW + 'Find end of requirements, delete redundant data...' + Fore.RESET)
            rent_list = list(filter(lambda rent: year(rent) == CRAWL_YEAR, rent_list))
            finish = True
        elif (y_start == CRAWL_YEAR + 1):
            print(Fore.YELLOW + 'No required data in this range' + Fore.RESET)
            rent_list = []
            break
        print_rentlist(rent_list, all_file)
        popular_list = list(filter(lambda rent: ispopular(rent), rent_list))
        print_rentlist(popular_list, popular_file)

    all_file.close()
    popular_file.close()

    end_time = time.time()
    print(Fore.GREEN + 'Used time: {} mins'.format((end_time - start_time) / 60) + Fore.RESET)


def push(start, end):
    start_time = time.time()

    match_url = []
    push_dict = {}
    boo_dict = {}
    all_like = all_boo = 0
    with open('all_articles.txt', 'r') as file:
        print(Fore.YELLOW + 'Open all_articles.txt, search for required date' + Fore.RESET)
        match_url = find_match_url(file, start, end)
        print(Fore.GREEN + '{} of url(s) found, start crawling...'.format(len(match_url)) + Fore.RESET)
    for idx, url in enumerate(match_url):
        r = get(url)
        if ((idx + 1) % 10 == 0 or idx + 1 == len(match_url)):
            print(Fore.YELLOW + 'Progress: {}/{}'.format(idx + 1, len(match_url)) + Fore.RESET)
        soup = BeautifulSoup(r.text, 'html.parser')
        push_list = soup.find_all(class_='push')
        for comment in push_list:
            user_id = comment.find(class_='push-userid')
            if user_id is None:
                continue
            user_id = user_id.string
            tag = comment.find(class_='push-tag')
            if (tag.string == '推 '):
                update_dict(user_id, push_dict)
                all_like += 1
            elif (tag.string == '噓 '):
                update_dict(user_id, boo_dict)
                all_boo += 1
    push_result = findtop10(push_dict)
    boo_result = findtop10(boo_dict)
    with open('push[{}-{}].txt'.format(start, end), 'w') as file:
        file.write('all like: {}\n'.format(all_like))
        file.write('all boo: {}\n'.format(all_boo))
        for idx, tup in enumerate(push_result):
            file.write('like #{}: {} {}\n'.format(idx + 1, tup[0], tup[1]))
        for idx, tup in enumerate(boo_result):
            file.write('boo #{}: {} {}\n'.format(idx + 1, tup[0], tup[1]))

    end_time = time.time()
    print(Fore.GREEN + 'Used time: {} mins'.format((end_time - start_time) / 60) + Fore.RESET)


def popular(start, end):
    start_time = time.time()

    match_url = []
    with open('all_popular.txt', 'r') as file:
        print(Fore.YELLOW + 'Open all_popular.txt, search for required date' + Fore.RESET)
        match_url = find_match_url(file, start, end)
        print(Fore.GREEN + '{} of url(s) found, start crawling...'.format(len(match_url)) + Fore.RESET)
    with open('popular[{}-{}].txt'.format(start, end), 'w') as file:
        file.write('number of popular articles: {}\n'.format(len(match_url)))
        for idx, url in enumerate(match_url):
            r = get(url)
            if ((idx + 1) % 10 == 0 or idx + 1 == len(match_url)):
                print(Fore.YELLOW + 'Progress: {}/{}'.format(idx + 1, len(match_url)) + Fore.RESET)
            soup = BeautifulSoup(r.text, 'html.parser')
            imgs_url = find_imgs_url(soup)
            for img_url in imgs_url:
                file.write(img_url + '\n')

    end_time = time.time()
    print(Fore.GREEN + 'Used time: {} mins'.format((end_time - start_time) / 60) + Fore.RESET)


def keyword(keyword, start, end):
    start_time = time.time()

    with open('all_articles.txt', 'r') as file:
        print(Fore.YELLOW + 'Open all_articles.txt, search for required date' + Fore.RESET)
        match_url = find_match_url(file, start, end)
        print(Fore.GREEN + '{} of url(s) found, start crawling...'.format(len(match_url)) + Fore.RESET)
    with open('keyword({})[{}-{}].txt'.format(keyword, start, end), 'w') as file:
        for idx, url in enumerate(match_url):
            r = get(url)
            if ((idx + 1) % 10 == 0 or idx + 1 == len(match_url)):
                print(Fore.YELLOW + 'Progress: {}/{}'.format(idx + 1, len(match_url)) + Fore.RESET)
            soup = BeautifulSoup(r.text, 'html.parser')
            content = soup.find(id='main-container').text

            if (not re.compile('※ 發信站').search(content)):
                print('could not found ※ 發信站 in ' + url)
                continue

            text = content.split('※ 發信站')[0]
            if (re.compile(keyword).search(text)):
                print(Fore.YELLOW + 'Find matching keyword' + Fore.RESET)
                imgs_url = find_imgs_url(soup)
                for img_url in imgs_url:
                    file.write(img_url + '\n')

    end_time = time.time()
    print(Fore.GREEN + 'Used time: {} mins'.format((end_time - start_time) / 60) + Fore.RESET)


def session():
    session = requests.Session()
    retry = Retry(
        total=6,
        backoff_factor=0.3,
        status_forcelist=(500, 502, 504, 521, 522),
    )
    adapter = HTTPAdapter(max_retries=retry)
    session.mount('http://', adapter)
    session.mount('https://', adapter)
    return session


def get(url):
    try:
        r = session().get(url)
        if (r.status_code == 200):
            print(Fore.GREEN + '[{}] GET {}'.format(r.status_code, r.url) + Fore.RESET)
            return r
        else:
            print(Fore.RED + '[{}] GET {}'.format(r.status_code, r.url))
            print('Exit...' + Fore.RESET)
            sys.exit(1)
    except Exception as ex:
        print(Fore.RED + 'Exception when GET: {}'.format(ex.__class__.__name__))
        print('Exit...' + Fore.RESET)
        sys.exit(1)


def previous_page_url(soup):
    btn = soup.find_all(class_='btn wide')[1]
    assert(btn.string == '‹ 上頁')
    return PTT_URL + btn.get('href')


def next_page_url(soup):
    btn = soup.find_all(class_='btn wide')[2]
    assert(btn.string == '下頁 ›')
    return PTT_URL + btn.get('href')


def valid_rule(rent):
    if rent.find('a') is None:
        return False
    elif (rent.find('a').string is None):
        if (url(rent) == PTT_URL + '/bbs/Beauty/M.1504367479.A.63D.html'):
            return True
    elif rent.find('a').string[0:4] == '[公告]':
        return False
    elif url(rent) in OMIT_URL:
        return False
    else:
        return True


def valid(rent_list):
    return list(filter(valid_rule, rent_list))


def title(rent):
    if (url(rent) == PTT_URL + '/bbs/Beauty/M.1504367479.A.63D.html'):
        return '[正妹] 乃木坂46@TGC'
    else:
        return rent.find('a').string


def url(rent):
    return PTT_URL + rent.find('a').get('href')


def year(rent):
    r = session().get(url(rent))
    soup = BeautifulSoup(r.text, 'html.parser')
    article_header = soup.find_all(class_='article-metaline')
    # FIXME: not consider articles that don't have article-meta-tag
    for header in article_header:
        if (header.find(class_='article-meta-tag').string == '時間'):
            return int(header.find(class_='article-meta-value').string.split(' ')[-1])


def date(rent):
    date = rent.find(class_='date').string
    return int(date[:2]) * 100 + int(date[3:5])


def ispopular(rent):
    span = rent.find(class_='nrec').find('span')
    if (span is not None and span.string == '爆'):
        return True
    else:
        return False


def print_rentlist(rent_list, file):
    for rent in rent_list:
        file.write('{},{},{}\n'.format(date(rent), title(rent), url(rent)))


def find_match_url(file, start, end):
    regex_date = re.compile(r'^\d+')
    regex_url = re.compile(r'https.*html')
    match_url = []
    for line in file:
        date = int(regex_date.search(line).group())
        url = regex_url.search(line).group()
        if (date >= start and date <= end):
            match_url.append(url)
        elif (date > end):
            break
    return match_url


def findtop10(dic):
    list1 = list(dic.items())
    list1 = sorted(list1, key=lambda tup: (-tup[1], tup[0]))
    if (len(list1) >= 10):
        list1 = list1[:10]
    return list1


def update_dict(user, dic):
    if (user in dic):
        dic[user] += 1
    else:
        dic[user] = 1


def find_imgs_url(soup):
    regex = re.compile(r'(\.jpg|\.jpeg|\.png|\.gif)$')
    urls_tag = soup.find(id='main-content').find_all('a')
    urls_list = [tag.string for tag in urls_tag if tag.string is not None]
    imgs_url = list(filter(regex.search, urls_list))
    valid_img_url = []
    for img_url in imgs_url:
        valid_img_url.append(img_url)
    return valid_img_url


def is_int(input):
    try:
        int(input)
    except ValueError:
        return False
    return True


def main():
    command = sys.argv
    if (len(command) == 2 and command[1] == 'crawl'):
        crawl()
    elif (len(command) == 4 and command[1] == 'push'):
        if (is_int(command[2]) and is_int(command[3])):
            push(int(command[2]), int(command[3]))
        else:
            print(Fore.RED + 'wrong date format' + Fore.RESET)
    elif (len(command) == 4 and command[1] == 'popular'):
        if (is_int(command[2]) and is_int(command[3])):
            popular(int(command[2]), int(command[3]))
        else:
            print(Fore.RED + 'wrong date format' + Fore.RESET)
    elif (len(command) == 5 and command[1] == 'keyword'):
        if (is_int(command[3]) and is_int(command[4])):
            keyword(command[2], int(command[3]), int(command[4]))
        else:
            print(Fore.RED + 'wrong date format' + Fore.RESET)
    else:
        print(Fore.RED + 'Worng input!')
        print('Usage: ')
        print('  python3 0750727.py crawl')
        print('  python3 0750727.py push start_date end_date')
        print('  python3 0750727.py popular start_date end_date')
        print('  python3 0750727.py keyword {keyword} start_date end_date' + Fore.RESET)


if __name__ == '__main__':
    main()
