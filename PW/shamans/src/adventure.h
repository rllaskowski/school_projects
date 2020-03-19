#ifndef SRC_ADVENTURE_H_
#define SRC_ADVENTURE_H_

#include <algorithm>
#include <vector>
#include <queue>
#include <stack>

#include "../third_party/threadpool/threadpool.h"

#include "./types.h"
#include "./utils.h"

typedef std::vector<Egg> vEgg_t;
typedef std::vector<Crystal> vCrystal_t;
typedef std::vector<GrainOfSand> vSand_t;

class Adventure {
public:
	virtual ~Adventure() = default;

	virtual uint64_t packEggs(std::vector<Egg> eggs, BottomlessBag& bag) = 0;

	virtual void arrangeSand(std::vector<GrainOfSand>& grains) = 0;

	virtual Crystal selectBestCrystal(std::vector<Crystal>& crystals) = 0;
};

class LonesomeAdventure : public Adventure {
public:
	LonesomeAdventure() {}

	virtual uint64_t packEggs(std::vector<Egg> eggs, BottomlessBag& bag);

	virtual void arrangeSand(std::vector<GrainOfSand>& grains);

	virtual Crystal selectBestCrystal(std::vector<Crystal>& crystals);
};

class TeamAdventure : public Adventure {
public:
	explicit TeamAdventure(uint64_t numberOfShamansArg)
	  : numberOfShamans(numberOfShamansArg), councilOfShamans(numberOfShamansArg) {}

	uint64_t packEggs(std::vector<Egg> eggs, BottomlessBag& bag);

	virtual void arrangeSand(std::vector<GrainOfSand>& grains);

	virtual Crystal selectBestCrystal(std::vector<Crystal>& crystals);

private:
	uint64_t numberOfShamans;
	ThreadPool councilOfShamans;
};


// Maximum element problem implementation
vCrystal_t::iterator selectBestCrystalTh(const vCrystal_t::iterator begin,
								  const vCrystal_t::iterator end) {
	if (begin == end) {
		throw std::runtime_error("0 sized vector of crystals");
	}

	auto best = begin;

	for (auto it = begin+1; it != end; ++it) {
		if (*best < *it) {
		  *best = *it;
		}
	}
	return best;
}

Crystal LonesomeAdventure::selectBestCrystal(std::vector<Crystal>& crystals) {
	return *selectBestCrystalTh(crystals.begin(), crystals.end());
}

Crystal TeamAdventure::selectBestCrystal(std::vector<Crystal>& crystals) {
	size_t avgWork = crystals.size()/this->numberOfShamans;
	size_t extraWork = crystals.size()%this->numberOfShamans;
	std::vector<std::future<vCrystal_t::iterator> > results;

	auto it = crystals.begin();

	for (size_t i = 0; i < this->numberOfShamans; ++i) {
		size_t work = avgWork;

		if (extraWork > 0) {
			++work;
			--extraWork;
		}
		if (work == 0) {
			break;
		}
		results.push_back(this->councilOfShamans.enqueue(selectBestCrystalTh, it, it+work));
		it += work;
	}

	auto best = results.begin()->get();

	for (auto it = results.begin()+1; it != results.end(); ++it) {
		auto tmp = it->get();

		if (*best < *tmp) {
			best = tmp;
		}
	}
	return *best;
}

// Backpack problem implementation
void packEggsTh(std::pair<uint64_t, bool> *row,
                std::pair<uint64_t, bool> *prevRow,
                size_t begin,
                size_t end,
                size_t currentRow,
                uint64_t size,
                uint64_t weight) {
	  for (size_t i = begin; i < end; ++i) {
    	row[i] = std::make_pair(prevRow[i].first, false);

    	if (i >= size && prevRow[i-size].first+weight > row[i].first) {
			row[i] = std::make_pair(prevRow[i-size].first+weight, true);
	   	}
	}
}

uint64_t LonesomeAdventure::packEggs(std::vector<Egg> eggs,
                                BottomlessBag& bag) {
	std::pair<uint64_t, bool> **results;
	results = new std::pair<uint64_t, bool> * [eggs.size()+1];

	for (size_t i = 0; i <= eggs.size(); i++) {
		results[i] = new std::pair<uint64_t, bool> [bag.getCapacity()+1];
	}

	for (size_t i = 0; i <= bag.getCapacity(); ++i) {
		results[0][i] = std::make_pair(0, false);
	}

	size_t currentRow = 0;
	for (auto egg = eggs.begin(); egg != eggs.end(); ++egg) {
		++currentRow;
		packEggsTh(results[currentRow], results[currentRow-1], 0,
		bag.getCapacity()+1, currentRow, egg->getSize(), egg->getWeight());
	}

	auto result = results[currentRow][bag.getCapacity()].first;

	uint64_t row = eggs.size();
	uint64_t col = bag.getCapacity();

	while (row != 0) {
		if (results[row][col].second) {
			bag.addEgg(eggs[row-1]);
			col -= eggs[row-1].getSize();
		}
		--row;
	}

	for (size_t i = 0; i <= eggs.size(); i++) {
		delete[] results[i];
	}
	delete[] results;

	return result;
}


uint64_t TeamAdventure::packEggs(std::vector<Egg> eggs, BottomlessBag& bag) {
	std::pair<uint64_t, bool> **results;
	results = new std::pair<uint64_t, bool> * [eggs.size()+1];

	for (size_t i = 0; i <= eggs.size(); i++) {
		results[i] = new std::pair<uint64_t, bool> [bag.getCapacity()+1];
	}

	for (size_t i = 0; i <= bag.getCapacity(); ++i) {
		results[0][i] = std::make_pair(0, false);
	}

	size_t currentRow = 0;
	size_t avgWork = (bag.getCapacity()+1)/this->numberOfShamans;
	std::queue<std::future<void> > waiters;

	for (auto egg = eggs.begin(); egg != eggs.end(); ++egg) {
		size_t extraWork = (bag.getCapacity()+1)%this->numberOfShamans;
		size_t begin = 0;

		++currentRow;

		for (size_t i = 0; i < this->numberOfShamans; ++i) {
			size_t work = avgWork;

			if (extraWork > 0) {
				--extraWork;
				++work;
			}

			if (work == 0) {
				break;
			}

			waiters.push(this->councilOfShamans.enqueue(packEggsTh,
			results[currentRow],
			results[currentRow-1],
			begin, begin+work, currentRow, egg->getSize(),
			egg->getWeight()));

			begin += work;
		}

		while (!waiters.empty()) {
			waiters.front().get();
			waiters.pop();
		}
	}

	auto result = results[currentRow][bag.getCapacity()].first;

	uint64_t row = eggs.size();
	uint64_t col = bag.getCapacity();

	while (row != 0) {
		if (results[row][col].second) {
			bag.addEgg(eggs[row-1]);
		  	col -= eggs[row-1].getSize();
		}
		--row;
	}

	for (size_t i = 0; i <= eggs.size(); i++) {
		delete[] results[i];
	}
	delete[] results;

	return result;
}

// Sort problem implementation
void merge(std::vector<GrainOfSand>::iterator begin,
            std::vector<GrainOfSand>::iterator mid,
            std::vector<GrainOfSand>::iterator end,
            std::vector<GrainOfSand>::iterator mergeMem) {
	std::vector<GrainOfSand>::iterator first = begin;
	std::vector<GrainOfSand>::iterator second = mid;
	std::vector<GrainOfSand>::iterator mergeMemBegin = mergeMem;

	while (first != mid && second != end) {
		if (*first < *second) {
			*(mergeMem++) = *(first++);
		} else {
			*(mergeMem++) = *(second++);
		}
	}

	while (first != mid) {
		*(mergeMem++) = *(first++);
	}

	while (second != end) {
		*(mergeMem++) = *(second++);
	}

	while (begin != end) {
		*(begin++) = *(mergeMemBegin++);
	}
}

void mergeSort(std::vector<GrainOfSand>::iterator begin,
	std::vector<GrainOfSand>::iterator end,
	std::vector<GrainOfSand>::iterator mergeMem) {
	size_t length = end-begin;

	if (length == 2 && *(begin+1) < *begin) {
		std::swap(*(begin+1), *begin);
	} else if (end-begin > 2) {
		std::vector<GrainOfSand>::iterator mid = begin+(length)/2;

		mergeSort(begin, mid, mergeMem);
		mergeSort(mid, end, mergeMem+(length)/2);
		merge(begin, begin+(length/2), end, mergeMem);
	}
}


void LonesomeAdventure::arrangeSand(std::vector<GrainOfSand>& grains) {
	std::vector<GrainOfSand> mergeMem(grains.size());

	mergeSort(grains.begin(), grains.end(), mergeMem.begin());
}

void mergeSortChunks(
		std::vector<std::pair<vSand_t::iterator,
		vSand_t::iterator> >::iterator begin,
		std::vector<std::pair<vSand_t::iterator,
		vSand_t::iterator> >::iterator end,
		vSand_t::iterator mergeMem,
		ThreadPool *threadPool) {
	size_t length = end-begin;

	if (length == 2) {
		merge(begin->first, begin->second, (begin+1)->second, mergeMem);
	} else if (length > 2) {
		std::vector<std::pair<vSand_t::iterator,
		vSand_t::iterator> >::iterator mid;
		mid = begin+(length/2);

		size_t leftChunksLength = mid->first-begin->first;

		mergeSortChunks(begin, mid, mergeMem, threadPool);
		threadPool->enqueue(mergeSortChunks, mid, end,
		mergeMem+leftChunksLength, threadPool).get();

		merge(begin->first, mid->first, (end-1)->second, mergeMem);
	}
}


void TeamAdventure::arrangeSand(std::vector<GrainOfSand>& grains) {
	vSand_t mergeMem(grains.size());

	size_t avgWork = grains.size()/this->numberOfShamans;
	size_t extraWork = grains.size()%this->numberOfShamans;

	std::vector<std::pair<vSand_t::iterator, vSand_t::iterator> > chunks;
	std::queue<std::future<void> >waiters;
	vSand_t::iterator memory = mergeMem.begin();
	vSand_t::iterator begin = grains.begin();

	for (size_t i = 0; i < this->numberOfShamans; ++i) {
		size_t work = avgWork;

		if (extraWork > 0) {
			++work;
			--extraWork;
		}

		if (work == 0) {
			break;
		}
		waiters.push(this->councilOfShamans.enqueue(mergeSort,begin,
				begin+work, memory));
		chunks.push_back(std::make_pair(begin, begin+work));

		memory += work;
		begin += work;
	}

	while (!waiters.empty()) {
		waiters.front().get();
		waiters.pop();
	}

	mergeSortChunks(chunks.begin(), chunks.end(),
	mergeMem.begin(), &this->councilOfShamans);
}

#endif  // SRC_ADVENTURE_H_
