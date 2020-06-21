#ifndef INDENTSTREAMBUF_H
#define INDENTSTREAMBUF_H

#include<iostream>
#include<string>
#include<vector>

using std::streambuf;
using std::fill_n;
using std::ostream;
using std::ostreambuf_iterator;

class IndentStreamBuf: public streambuf {
public:
    IndentStreamBuf(streambuf* sbuf): m_sbuf{sbuf}, level{0}, first{true} {} 
    void indent() {level += 4;}
    void unindent() { if(level >= 4) level-= 4; }
protected:
    virtual int_type overflow(int_type c) {
        if (traits_type::eq_int_type(c, traits_type::eof()))
            return m_sbuf->sputc(c);
        
        if (first){
            fill_n(ostreambuf_iterator<char>(m_sbuf), level, ' ');
            first = false;
        }

        if (traits_type::eq_int_type(m_sbuf->sputc(c), traits_type::eof()))
            return traits_type::eof();

        if (traits_type::eq_int_type(c, traits_type::to_char_type('\n')))
            first = true;

        return traits_type::not_eof(c);
    }

    streambuf* m_sbuf;
    int level;
    bool first;
};

class IndentStream: public ostream {
public:
    IndentStream(ostream& os): buf{os.rdbuf()}, ostream{&buf} {}
    friend ostream& unindent(ostream& stream);
    friend ostream& indent(ostream& stream);
private:
    IndentStreamBuf buf;
};

ostream& unindent(ostream& stream){
    IndentStream* pIndentStream = dynamic_cast<IndentStream*>(&stream);
    if (pIndentStream != nullptr){
        pIndentStream->buf.unindent();
    }
    return stream;
}

ostream& indent(ostream& stream){
    IndentStream* pIndentStream = dynamic_cast<IndentStream*>(&stream);
    if (pIndentStream != nullptr){
        pIndentStream->buf.indent();
    }
    return stream;
}

#endif