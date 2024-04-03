#include <algorithm>
#include <cmath>
#include <iostream>
#include <map>
#include <set>
#include <string>
#include <utility>
#include <vector>

using namespace std;

const int MAX_RESULT_DOCUMENT_COUNT = 5;

string ReadLine() {
    string s;
    getline(cin, s);
    return s;
}

int ReadLineWithNumber() {
    int result = 0;
    cin >> result;
    ReadLine();
    return result;
}

vector<string> SplitIntoWords(const string& text) {
    vector<string> words;
    string word;
    for (const char c : text) {
        if (c == ' ') {
            if (!word.empty()) {
                words.push_back(word);
                word.clear();
            }
        } else {
            word += c;
        }
    }
    if (!word.empty()) {
        words.push_back(word);
    }

    return words;
}

struct Document {
    int id;
    double relevance;
};

class SearchServer {
public:
    void SetStopWords(const string& text) {
        for (const string& word : SplitIntoWords(text)) {
            stop_words_.insert(word);
        }
    }

    void AddDocument(int document_id, const string& document) {
        const vector<string> words = SplitIntoWordsNoStop(document);
        double tf = 0;
        for (auto& word: words) {
            double countw = count(words.begin(), words.end(), word);
            tf = countw / words.size();
            documents_[word].insert({document_id, tf});
        }
    }

    vector<Document> FindTopDocuments(const string& raw_query) const {
        Query query_words = ParseQuery(raw_query);
        auto matched_documents = FindAllDocuments(query_words);

        sort(matched_documents.begin(), matched_documents.end(),
             [](const Document& lhs, const Document& rhs) {
                 return lhs.relevance > rhs.relevance;
             });
        if (matched_documents.size() > MAX_RESULT_DOCUMENT_COUNT) {
            matched_documents.resize(MAX_RESULT_DOCUMENT_COUNT);
        }
        return matched_documents;
    }

    void setDocument_count(int newDocument_count);
    int document_count() const;

private:
    using  DocumentContent = map<string, map<int, double>>; // map<word, map<id, tf>>
    int document_count_ = 0;
    struct Query {
        set<string> query_words;
        set<string> minus_words;
    };

    DocumentContent documents_;

    set<string> stop_words_;

    bool IsStopWord(const string& word) const {
        return stop_words_.count(word) > 0;
    }

    vector<string> SplitIntoWordsNoStop(const string& text) const {
        vector<string> words;
        for (const string& word : SplitIntoWords(text)) {
            if (!IsStopWord(word)) {
                words.push_back(word);
            }
        }
        return words;
    }

    // first - query_words
    // second - minus_words
    Query ParseQuery(const string& text) const {
        set<string> query_words;
        set<string> minus_words;
        for (const string& word : SplitIntoWordsNoStop(text)) {
            if (word[0] == '-')
            {
                minus_words.insert(word.substr(1));
            }
            else
            {
                query_words.insert(word);
            }
        }
        return Query{query_words, minus_words};
    }

    vector<Document> FindAllDocuments(const Query& query_words) const {
        vector<Document> matched_documents;
            const map<int, double> relevance = MatchDocument(documents_, query_words);
            for (const auto& doc : relevance) {
                matched_documents.push_back({doc.first, doc.second});
            }
        return matched_documents;
    }

    map<int, double> MatchDocument(const DocumentContent& content, const Query& query) const {
        map<int, double>  matched_result; // id, relevance
        if (query.query_words.empty()) {
            return matched_result;
        }
        for (const string& w: query.query_words) {
            int count = content.count(w); // count of docs that contains word
            if (!count) continue;

            double idf = log((double)document_count_ / content.at(w).size());
            for (const auto& [id, tf]: content.at(w)) {
                matched_result[id] += tf * idf;
            }
        }
        return matched_result;
    }
};

SearchServer CreateSearchServer() {
    SearchServer search_server;
    search_server.SetStopWords(ReadLine());

    search_server.setDocument_count(ReadLineWithNumber());
    for (int document_id = 0; document_id < search_server.document_count(); ++document_id) {
        search_server.AddDocument(document_id, ReadLine());
    }

    return search_server;
}

int main() {
    const SearchServer search_server = CreateSearchServer();

    const string query = ReadLine();
    for (const auto& [document_id, relevance] : search_server.FindTopDocuments(query)) {
        cout << "{ document_id = "s << document_id << ", "
             << "relevance = "s << relevance << " }"s << endl;
    }
}

void SearchServer::setDocument_count(int newDocument_count)
{
    document_count_ = newDocument_count;
}

int SearchServer::document_count() const
{
    return document_count_;
}
