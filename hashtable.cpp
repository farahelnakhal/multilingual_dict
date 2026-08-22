#include "hashtable.h"
#include <fstream>
#include <sstream>
#include <algorithm>
#include <cctype>
#include <iostream>

using namespace std;

//============================================================================
//helper function to convert string to lowercase and trim whitespaces
string format(const string& str) {
    string result;
    
    //process each character in the input string one by one
    bool lastWasSpace = true;
    
    for (char c : str) {
        if (isspace(c)) {
            //current character is whitespace
            if (!lastWasSpace) {
                //only add space if previous character was not space
                result += ' ';
                lastWasSpace = true;
            }
        } else {
            //current character is not whitespace
            result += tolower(c);
            lastWasSpace = false;
        }
    }
    
    //if original string ended with spaces after a word
    if (!result.empty() && result.back() == ' ') {
        result.pop_back();  //remove the trailing space
    }
    
    return result;
}

//============================================================================
//helper function to print language name properly
string formatLanguageName(const string& lang) {
    if (lang.empty()) return lang;
    string formatted = lang;
    formatted[0] = toupper(formatted[0]); //convert first character to uppercase
    //convert rest to lowercase
    for (size_t i = 1; i < formatted.size(); i++) {
        formatted[i] = tolower(formatted[i]);
    }
    return formatted;
}
//============================================================================
//translation class implementation
Translation::Translation(string meanings, string language) {
    this->language = format(language); //format language then add
    
    //split meanings by semicolon and add to the meanings vector
    stringstream ss(meanings);
    string meaning;
    while (getline(ss, meaning, ';')) {
        //trim whitespace and add meaning if not empty
        meaning = format(meaning);
        if (!meaning.empty()) {
            this->meanings.push_back(meaning);
        }
    }
}

//============================================================================
//method to add meaning of an already existent word
void Translation::addMeaning(string newMeanings) {
    stringstream ss(newMeanings);
    string meaning;
    while (getline(ss, meaning, ';')) {
        //trim whitespace
        meaning = format(meaning);
        if (!meaning.empty()) {
            //check if meaning already exists
            bool found = false;
            for (int i = 0; i < meanings.size(); i++) {
                if (meanings[i] == meaning) {
                    found = true;
                    break;
                }
            }
            //if not found add a new one
            if (!found) {
                meanings.push_back(meaning);
            }
        }
    }
}

//============================================================================
//entry class implementation
Entry::Entry(string word, string meanings, string language) : deleted(false) {
    this->word = format(word);
    translations.push_back(Translation(meanings, language));
}

//============================================================================
//method to add a translation to a language
void Entry::addTranslation(string newMeanings, string language) {
    string langLower = format(language);
    
    //check if translation for this language already exists
    for (auto& translation : translations) {
        if (translation.language == langLower) {
            translation.addMeaning(newMeanings);
            return;
        }
    }
    
    //if not create new translation
    translations.push_back(Translation(newMeanings, language));
}

//============================================================================
//method to print translation
void Entry::print() {
    //iterate over all translations
    for (int t = 0; t < translations.size(); t++) {
        cout << formatLanguageName(translations[t].language) << " : "; //format langauge name
        //print each meaning of the word, seperated by ;
        for (int i = 0; i < translations[t].meanings.size(); i++) {
            cout << translations[t].meanings[i];
            if (i != translations[t].meanings.size() - 1) {
                cout << "; ";
            }
        }
        cout << endl; //after language ends start a new line
    }
}

//============================================================================
//hashtable class implementation
HashTable::HashTable(int capacity) : size(0), capacity(capacity), collisions(0) {
    buckets = new Entry*[capacity]; //dynamically allocate new entry buckets
    //all elements should point to null for now
    for (int i = 0; i < capacity; i++) {
        buckets[i] = nullptr;
    }
}

//============================================================================
//hashtable destructor
HashTable::~HashTable() {
    //go over ever item in everybucket and delete it
    for (int i = 0; i < capacity; i++) {
        if (buckets[i] != nullptr) {
            delete buckets[i];
        }
    }
    delete[] buckets; //delete the buckets
}

//============================================================================
//convert string to integer
unsigned long HashTable::hashCode(string word) {
//polynomial rolliong hashing
//     const int p = 31;
//     unsigned long hash_value = 0;
//     unsigned long p_pow = 1;
//     string lowerWord = format(word);
    
//     for (size_t i = 0; i < lowerWord.length(); i++) {
//         char c = lowerWord[i];
//         hash_value = (hash_value + (c - 'a' + 1) * p_pow);
//         p_pow = (p_pow * p);
//     }
    
//     return hash_value % capacity;
// }
//----------------------------------------------------------------------------
// cyclic shift hashing
//     unsigned long hash_value = 0;
//     string lowerWord = format(word);
    
//     for (size_t i = 0; i < lowerWord.length(); i++) {
//         char c = lowerWord[i];
//         hash_value = (hash_value << 5) | (hash_value >> (sizeof(hash_value)*8 - 5));
//         hash_value ^= c;
//     }
    
//     return hash_value % capacity;
// }

//----------------------------------------------------------------------------
//FNV-1a hashing
    const unsigned long FNV_offset_basis = 14695981039346656037UL;
    const unsigned long FNV_prime = 1099511628211UL;
    
    unsigned long hash = FNV_offset_basis;
    string lowerWord = format(word);
    
    for (char c : lowerWord) {
        hash ^= static_cast<unsigned long>(c);
        hash *= FNV_prime;
    }
    
    return hash % capacity;
}

//============================================================================
//gets size of table
unsigned int HashTable::getSize() {
    return size;
}

//============================================================================
//returns collisions caused in table
unsigned int HashTable::getCollisions() {
    return collisions;
}

//============================================================================
//imports data from a file
void HashTable::import(string path) {
    //error detection
    ifstream file(path);
    if (!file.is_open()) {
        cout << "Error: Could not open file " << path << endl;
        return;
    }
    
    string language;
    getline(file, language); //first line is the language
    
    string line;
    int count = 0;
    int skipped = 0;
    while (getline(file, line)) {
        //skip empty lines
        if (line.empty()) {
            skipped++; //increment counter
            continue;
        }
        
        int colon_pos = line.find(':');//to split def and meaning
        string word = line.substr(0, colon_pos); //word from 0 to :
        string meanings = line.substr(colon_pos + 1); //meaning from :+1
        
        //trim whitespace and lowerspace everything
        word = format(word);
        meanings = format(meanings);

        //if for,atted incorrectly or empty skip and increment counter
        if (word.empty() || meanings.empty() || colon_pos == string::npos) {
            cout << "Skipped line: " << line << endl;
            skipped++;
            continue;
        }
        
        insert(word, meanings, language); //insert data
        count++; //increment count
    }
    
    //print confirmation message
    cout << count << " " << language << " words imported successfully. Skipped " << skipped << " lines." << endl;
}


//============================================================================
//inserts data to hashtable
void HashTable::insert(string word, string meanings, string language) {
    unsigned long index = hashCode(word); //get index by hashing the word
    unsigned long original_index = index;
    string lowerWord = format(word); //format the word
    
    while (true) {
        //Case 1: bucket is empty or deleted
        if (buckets[index] == nullptr || buckets[index]->deleted) {
            buckets[index] = new Entry(word, meanings, language); //create new
            size++;
            return;
        }
        
        //Case 2: found existing entry with same word
        if (format(buckets[index]->word) == lowerWord) {
            buckets[index]->addTranslation(meanings, language); //add to old
            return;
        }
        
        //Case 3: collision (bucket is occupied by different key)
        collisions++; //increment collisions
        index = (index + 1) % capacity; //linear probing
        
        //check if have searched entire table
        if (index == original_index) {
            cout << "Error: Hash table is full." << endl; //give error message
            cout << size << "\n" << index << endl;

            return;
        }
    }
}

//============================================================================
//deletes word from table
void HashTable::delWord(string word) {
    unsigned long index = hashCode(word); //get index by hashing the word
    int original_index = index;
    int probe = 1; //probe counter
    string lowerWord = format(word); //formats word
    
    //linear probing to find the word
    while (buckets[index] != nullptr) {
        if (!buckets[index]->deleted && format(buckets[index]->word) == lowerWord) {
            //mark as deleted (lazy deletion)
            buckets[index]->deleted = true;
            size--;
            cout << word << " has been successfully deleted from the Dictionary." << endl; //confirmation messgae
            return;
        }
        
        index = (index + 1) % capacity; //linear probing
        probe++;
        
        //if too much stop
        if (probe > capacity || index == original_index) {
            break;
        }
    }
    
    cout << word << " not found in the Dictionary." << endl; //print error message
}

//============================================================================
//deletes one translation
void HashTable::delTranslation(string word, string language) {
    unsigned long index = hashCode(word); //get index by hashing the word
    int original_index = index;
    int probe = 1;
    string lowerWord = format(word); //format word
    string lowerLang = format(language); //format language name
    
    //linear probing to find the word
    while (buckets[index] != nullptr) {
        if (!buckets[index]->deleted && format(buckets[index]->word) == lowerWord) {
            //find the translation with matching language
            auto& translations = buckets[index]->translations;
            for (auto it = translations.begin(); it != translations.end(); ++it) {
                if (it->language == lowerLang) {
                    translations.erase(it); //remove 
                    //print confirmation messagee
                    cout << "Translation has been successfully deleted from the Dictionary." << endl;
                    
                    // If no translations left, delete the entire entry
                    if (translations.empty()) {
                        delWord(word);
                    }
                    return;
                }
            }
            //print error message
            cout << "Translation not found for " << word << " in " << language << endl;
            return;
        }
        
        index = (index + 1) % capacity; //linear probing
        probe++;
        
        //if too much and not found stop
        if (probe > capacity || index == original_index) {
            break;
        }
    }
    
    cout << word << " not found in the Dictionary." << endl; //output error message
}

//============================================================================
//deletes specific meaning
void HashTable::delMeaning(string word, string meaning, string language) {
    unsigned long index = hashCode(word); //hash word to find index
    int original_index = index;
    int probe = 1;
    string lowerWord = format(word); //format word
    string lowerLang = format(language); //format language
    
    //linear probing to find  word
    while (buckets[index] != nullptr) {
        if (!buckets[index]->deleted && format(buckets[index]->word) == lowerWord) {
            //find  translation with matching language
            for (auto& translation : buckets[index]->translations) {
                if (translation.language == lowerLang) {
                    //Find meaning to delete
                    auto& meanings = translation.meanings;
                    for (auto it = meanings.begin(); it != meanings.end(); ++it) {
                        if (*it == meaning) {
                            meanings.erase(it); //remove
                            //print confirmation
                            cout << "Meaning has been successfully deleted from the Dictionary." << endl;
                            
                            //if no meanings left delete entire translation
                            if (meanings.empty()) {
                                delTranslation(word, language);
                            }
                            return;
                        }
                    }
                    //print error message
                    cout << "Meaning not found for " << word << " in " << language << endl;
                    return;
                }
            }
            //print error message
            cout << "Translation not found for " << word << " in " << language << endl;
            return;
        }
        
        index = (index + 1) % capacity; //linear probing
        probe++;
        
        //if surpassed capacity stop
        if (probe > capacity || index == original_index) {
            break;
        }
    }
    
    cout << word << " not found in the Dictionary." << endl; //print error message
}

//============================================================================
//exports data to a file
void HashTable::exportData(string language, string filePath) {
    ofstream file(filePath);
    
    //error message if can't create file
    if (!file.is_open()) {
        cout << "Error: Could not create file " << filePath << endl;
        return;
    }
    
    file << formatLanguageName(language) << endl; //format language name
    
    int count = 0;
    string lowerLang = format(language); //format langauge
    //iterate over it
    for (int i = 0; i < capacity; i++) {
        if (buckets[i] != nullptr && !buckets[i]->deleted) { //if there is data in buckey
            //go over every word and write the word
            for (size_t t = 0; t < buckets[i]->translations.size(); t++) {
                if (buckets[i]->translations[t].language == lowerLang) { 
                    file << buckets[i]->word << ":"; // put : after word
                    for (size_t j = 0; j < buckets[i]->translations[t].meanings.size(); j++) {
                        file << buckets[i]->translations[t].meanings[j];
                        //if more than one meaning seperate by ;
                        if (j != buckets[i]->translations[t].meanings.size() - 1) {
                            file << ";";
                        }
                    }
                    file << endl; //new line after every word
                    count++; //increment count
                    break;
                }
            }
        }
    }
    
    cout << count << " records have been successfully exported to " << filePath << endl; //print success message
}

//============================================================================
//finds a specific word
void HashTable::find(string word) {
    unsigned long index = hashCode(word); //turns word to index
    int original_index = index;
    int comparisons = 1; //counter for comparisions
    string lowerWord = format(word); //format word for comparision
    
    //linear probing to find word
    while (buckets[index] != nullptr) {
        if (!buckets[index]->deleted && format(buckets[index]->word) == lowerWord) {
            //print confirmation message and numebr of comparisions
            cout << word << " found in the Dictionary after " << comparisons << " comparisons." << endl;
            buckets[index]->print(); //print meaning
            return;
        }
        
        index = (index + 1) % capacity; //linear probing
        comparisons++;
        
        //if already passed capacity then its not here
        if (comparisons > capacity || index == original_index) {
            break;
        }
    }
    
    //print error message
    cout << word << " not found in the Dictionary." << endl;
}