#include "pch.h"
#include "base64lib.h"

void cli::GroupsEncode(pair_us data, prioque & keep_order)
{
	std::string trans_data;
	unsigned prev[6] = { 0u };
	unsigned i = 0u;
	for (; i < 3 && data.second[i]; i++)
	{
		unsigned inx = int(data.second[i]);
		unsigned pre_bit[6] = { 0u };

		for (unsigned k = 0; k < 2 * (i + 1); k++, inx >>= 1)
			pre_bit[k] = inx & 1;
		if (i > 0u) {
			for (unsigned k = 0; k < 2 * i; k++)
				inx += prev[k] << (5 - k);
		}
		assert(inx < 65);
		trans_data += base64keys[inx];
		for (unsigned k = 0; k < 2 * (i + 1); k++)
			prev[2 * i - k + 1] = pre_bit[k];
	}
	if (i > 0) {
		unsigned inx = 0u;
		for (unsigned k = 0; k < 2 * i; k++)
			inx += prev[k] << (5 - k);
		assert(inx < 65);
		trans_data += base64keys[inx];
		while (i < 3) trans_data += '=', i++;
	}
	data.second = trans_data;
	keep_order.push(std::move(data));
}

void cli::GroupsDecode(pair_us data, prioque & keep_order)
{
	Base64Table v;
	std::string tran_data;
	unsigned suffix_bit[6] = { 0u };

	unsigned y1 = v[data.second[0]] << 2, w1 = v[data.second[1]];
	for (unsigned k = 0u; k < 4; k++, w1 >>= 1)
		suffix_bit[k] = w1 & 1;
	unsigned x1 = y1 + w1;
	tran_data += char(x1);

	unsigned y2 = 0u, w2 = data.second[2] == '=' ? 0u : v[data.second[2]];
	for (unsigned k = 0u; k < 4; k++)
		y2 += suffix_bit[4 - k - 1] << (7 - k);
	for (unsigned k = 0u; k < 2; k++, w2 >>= 1)
		suffix_bit[k] = w2 & 1;
	unsigned x2 = y2 + w2;
	tran_data += char(x2);

	unsigned y3 = 0u, w3 = data.second[3] == '=' ? 0u : v[data.second[3]];
	for (unsigned k = 0u; k < 2; k++)
		y3 += suffix_bit[2 - k - 1] << (7 - k);
	unsigned x3 = y3 + w3;
	tran_data += char(x3);
	data.second = tran_data;
	keep_order.push(std::move(data));
}

std::string cli::Base64::encode_or_decode(const char * cpstr, CodecType valueType, bool is_open, std::size_t threads_num)
{
	prioque keep_order;
	std::queue<pair_us> que;
	std::vector<std::thread> threads;
	pair_us p;
	std::string bucket;

	for (unsigned w = 0u; *cpstr; w++)
	{
		p.second.clear();
		for (unsigned i = 0u; i < (3 + (valueType & 1)) && *cpstr; i++, cpstr++)
			p.second += *cpstr;
		p.first = w;
		que.push(p);
	}

	while (!que.empty())
	{
		for (unsigned i = 0; i < threads_num; i++)
		{
			if (!que.empty())
			{
				pair_us data(que.front().first, que.front().second);
				que.pop();
				if (is_open)
				{
					if (valueType == E_ENCODE)
						threads.push_back(std::thread(GroupsEncode, std::ref(data), std::ref(keep_order)));
					else
						threads.push_back(std::thread(GroupsDecode, std::ref(data), std::ref(keep_order)));
				}
				else
				{
					if (valueType == E_ENCODE)
						GroupsEncode(data, keep_order);
					else
						GroupsDecode(data, keep_order);
				}
			}
		}
		if (is_open)
		{
			for (auto & thread : threads) thread.join();
			threads.clear();
		}
	}

	while (!keep_order.empty()) bucket += keep_order.top().second, keep_order.pop();
	return bucket;
}
