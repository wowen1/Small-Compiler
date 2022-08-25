//William Owen, CSE 340, Professor Bazzi.
//ASU ID: 1207961816

#include <iostream>
#include <cstdlib>
#include <stdio.h>
#include <stdlib.h>
#include <stdarg.h>
#include <ctype.h>
#include <string.h>
#include "compiler.h"
#include "lexer.h"
#include <vector>

using namespace std;


InstructionNode* parse_program();
void parse_var_section();
void parse_id_list();
InstructionNode* parse_body();
InstructionNode* parse_stmt_list();
InstructionNode* parse_stmt();
InstructionNode* parse_assign_stmt();
InstructionNode* parse_expr(string s);
Token parse_op();
Token parse_primary();
InstructionNode* parse_output_stmt();
InstructionNode* parse_input_stmt();
InstructionNode* parse_while_stmt();
InstructionNode* parse_condition();
Token parse_relop();
InstructionNode* parse_if_stmt();
InstructionNode*  parse_switch_stmt();
InstructionNode* parse_case_list(InstructionNode* noop, Token var);
InstructionNode* parse_case(InstructionNode* noop, Token var);
InstructionNode*  parse_default_case(InstructionNode* noop);
InstructionNode* parse_for_stmt();
void parse_inputs();
void parse_num_list();
int findAddress(string s);
void location (string s);

LexicalAnalyzer lexer;

struct varLocations
{
    string varName;
    int addr;
};

vector<varLocations> locationTable; //location table


//This function takes a lexeme and assigns it a location in memory.
void location (string s)
{
    varLocations* var = new varLocations;
    var->addr = next_available;
    var->varName = s;
    next_available++;

    mem[var->addr] = 0;  //initialized to 0

    locationTable.push_back(*var);
}

//starts the show
struct InstructionNode * parse_generate_intermediate_representation()
{
    parse_program();
}

//outputs error message if syntax error.
void syntax_error()
{
    cout << "SYNTAX ERROR\n";
    exit(1);
}

//
Token expect(TokenType expected_type)
{
    Token t = lexer.GetToken();
    if (t.token_type != expected_type)
        syntax_error();
    return t;
}

//This function starts parsing the program.
InstructionNode* parse_program()
{
    InstructionNode *instructs = new InstructionNode;
    parse_var_section();
    instructs = parse_body();

    InstructionNode *temp = new InstructionNode;
    temp = instructs;

    parse_inputs();

    return instructs;
}
//This function starts parsing the variable section of the program.
void parse_var_section()
{
    Token t;

    parse_id_list();

    t = expect(SEMICOLON);

}
//This function parses the variable list at the start of a program.
void parse_id_list()
{
    Token t,t2;

    t = expect(ID);


    location(t.lexeme);

    t2 = lexer.peek(1);
    if(t2.token_type == COMMA)
    {
        t2 = expect(COMMA);
        parse_id_list();
    }
    else if(t2.token_type == SEMICOLON)
    {
        return;
    }
    else
    {
        syntax_error();
    }
}
//This function parses the body of a program and other instructions (if,while,for, etc)
InstructionNode* parse_body()
{
    Token t;
    InstructionNode* inst = new InstructionNode;
    t = expect(LBRACE);
    inst = parse_stmt_list();
    t = expect(RBRACE);
    return inst;
}
//This function generates an instruction list by parsing the body of a program.
InstructionNode* parse_stmt_list()
{
    Token t;

    InstructionNode *inst1 = new InstructionNode;
    InstructionNode *instList = new InstructionNode;

   inst1 = parse_stmt();


    t = lexer.peek(1);
    if(t.token_type == RBRACE)
    {
        //return inst1;
    }
    else
    {
        instList = parse_stmt_list();


        InstructionNode* temp;// = new InstructionNode;

        temp = inst1;
        while(temp->next != nullptr)
        {
            temp = temp->next;
        }
        temp->next = instList;


    }
    /*if(inst1->next != nullptr)
    {
     //cout<<"AAAAINST NEXT TYPE IS = "<<inst1->next->type<<endl;
    }*/
    return inst1;
}

//This function parses a single statement of a program and returns it to the statement list.
InstructionNode* parse_stmt()
{
    Token t;
    t = lexer.peek(1);

    InstructionNode *inst = new InstructionNode;

    if(t.token_type == ID)
    {
        inst = parse_assign_stmt();
    }
    else if (t.token_type == WHILE)
    {
        inst = parse_while_stmt();
    }
    else if(t.token_type == IF)
    {
        inst = parse_if_stmt();
    }
    else if(t.token_type == SWITCH)
    {
        inst = parse_switch_stmt();
    }
    else if(t.token_type == FOR)
    {
        inst = parse_for_stmt();
    }
    else if(t.token_type == OUTPUT)
    {
        inst = parse_output_stmt();
    }
    else if(t.token_type == INPUT)
    {
        inst = parse_input_stmt();
    }
   return inst;
}

//This function parses an assignment statement.
InstructionNode* parse_assign_stmt()
{
    Token t,t2;
    string s;
    t = expect(ID);
    s = t.lexeme;
    t = expect(EQUAL);

    InstructionNode *inst = new InstructionNode;

    t2 = lexer.peek(2);
    if(t2.token_type == PLUS || t2.token_type == MINUS || t2.token_type == MULT || t2.token_type == DIV)
    {
        inst = parse_expr(s);
    }
    else
    {
        inst->type = ASSIGN;
        int addr;
        addr = findAddress(s);
        inst->assign_inst.left_hand_side_index = addr;

        t = parse_primary();
        addr = findAddress(t.lexeme);
        inst->assign_inst.operand1_index = addr;

        inst->assign_inst.op = OPERATOR_NONE;
        inst->next = nullptr;
    }
    t = expect(SEMICOLON);


    return inst;
}

//This function parses an expression that can be tied to statements.
InstructionNode* parse_expr(string s)
{
    Token t;
    int addr;
    InstructionNode *inst = new InstructionNode;
    inst->type = ASSIGN;

    addr = findAddress(s);
    inst->assign_inst.left_hand_side_index = addr;

    t = parse_primary();
    addr = findAddress(t.lexeme);
    inst->assign_inst.operand1_index = addr;

    t = parse_op();
    if(t.token_type == PLUS)
    {
        inst->assign_inst.op = OPERATOR_PLUS;
    }
    else if(t.token_type == MINUS)
    {
        inst->assign_inst.op = OPERATOR_MINUS;
    }
    else if(t.token_type == MULT)
    {
        inst->assign_inst.op = OPERATOR_MULT;
    }
    else if(t.token_type == DIV)
    {
        inst->assign_inst.op = OPERATOR_DIV;
    }

    t = parse_primary();
    addr = findAddress(t.lexeme);
    inst->assign_inst.operand2_index = addr;

    inst->next = nullptr;

    return inst;
}

//This function parses an arithmetic operator for a statement.
Token parse_op()
{
    Token t;
    t = lexer.peek(1);
    if(t.token_type == PLUS)
    {
        t = expect(PLUS);
    }
    else if(t.token_type == MINUS)
    {
        t = expect(MINUS);
    }
    else if(t.token_type == MULT)
    {
        t = expect(MULT);
    }
    else if(t.token_type == DIV)
    {
        t = expect(DIV);
    }
    else
    {
        syntax_error(); //scream
    }
    return t;
}

//This function parses a variable or a constant for a statement.
Token parse_primary()
{
    Token t;
    t = lexer.peek(1);
    int addr;
    if(t.token_type == ID)
    {
        t = expect(ID);
        return t;
    }
    else if(t.token_type == NUM)
    {
        t = expect(NUM);
        location(t.lexeme);
        addr = findAddress(t.lexeme);
        if(addr != -1)
        {
            mem[addr] = stoi(t.lexeme);
        }
        return t;
    }
    else
    {
        syntax_error();
    }
}

//This function parses an output statement.
InstructionNode* parse_output_stmt()
{
    Token t;
    int addr;
    InstructionNode* inst = new InstructionNode;
    inst->type = OUT;
    t = expect(OUTPUT);

    t = expect(ID);
    addr = findAddress(t.lexeme);
    inst->output_inst.var_index = addr;

    t = expect(SEMICOLON);

    inst->next = nullptr;

    return inst;
}

//This function parses an input statement.
InstructionNode* parse_input_stmt()
{
    Token t;
    int addr;
    InstructionNode* inst = new InstructionNode;
    inst->type = IN;
    t = expect(INPUT);

    t = expect(ID);
    addr = findAddress(t.lexeme);
    inst->input_inst.var_index = addr;

    t = expect(SEMICOLON);

    inst->next = nullptr;

    return inst;
}

//This function parses an while loop statement and the body of the while loop.
InstructionNode* parse_while_stmt()
{
    Token t;
    t = expect(WHILE);

    InstructionNode *inst = new InstructionNode;

    inst = parse_condition();
    inst->next = parse_body();

    InstructionNode *jmp = new InstructionNode;
    jmp->type = JMP;
    jmp->jmp_inst.target = inst;

    InstructionNode *temp = new InstructionNode;
    temp = inst;
    while(temp->next != nullptr)
    {
        temp = temp->next;
    }
    temp->next = jmp;

    InstructionNode *noop = new InstructionNode;
    noop->type = NOOP;
    noop->next = nullptr;

    jmp->next = noop;

    inst->cjmp_inst.target = noop;

    return inst;
}

//This function parses the condition for any conditional statements.
InstructionNode* parse_condition()
{
    int addr;
    Token t;
    InstructionNode *inst = new InstructionNode;

    inst->type = CJMP;

    t = parse_primary();
    addr = findAddress(t.lexeme);
    inst->cjmp_inst.operand1_index = addr;


    t = parse_relop();
    if(t.token_type == GREATER)
    {
        inst->cjmp_inst.condition_op = CONDITION_GREATER;
    }
    else if(t.token_type == LESS)
    {
        inst->cjmp_inst.condition_op = CONDITION_LESS;
    }
    else
    {
        inst->cjmp_inst.condition_op = CONDITION_NOTEQUAL;
    }

    t = parse_primary();
    addr = findAddress(t.lexeme);
    inst->cjmp_inst.operand2_index = addr;

    return inst;
}

//This function parses a Conditional Operator for use in a conditional statement.
Token parse_relop()
{
    Token t;
    t = lexer.peek(1);
    if(t.token_type == GREATER)
    {
        t = expect(GREATER);
    }
    else if(t.token_type == LESS)
    {
        t = expect(LESS);
    }
    else if(t.token_type == NOTEQUAL)
    {
        t = expect(NOTEQUAL);
    }
    else
    {
        syntax_error();
    }
    return t;
}

//This function takes a variable or a constant and returns the address in memory.
int findAddress(string s)
{
    int i;
    int addr;
    addr = -1;
    for ( i = 0; i < locationTable.size(); i++)
    {
        if(s == locationTable[i].varName)
        {
            addr = locationTable[i].addr;
        }
    }
    return addr;
}

//This function parses an IF-statement and the body of the IF-statement
InstructionNode* parse_if_stmt()
{
    Token t;
    t = expect(IF);

    InstructionNode *inst = new InstructionNode;

    inst = parse_condition(); //parse condition of if-statement
    inst->next = parse_body(); //parse body of if-statement


    InstructionNode *noop = new InstructionNode; //create noop node
    noop->type = NOOP;
    noop->next = nullptr;

    InstructionNode *temp = new InstructionNode;
    temp = inst; //body of if-statement

    while(temp->next != nullptr)
    {
        temp = temp->next;
    }
    temp->next = noop; //append noop to body of if-statement
    inst->cjmp_inst.target = noop; //append noop to cjmp jump target

    return inst;
}

//This function parses a switch statement and the body of each case of the switch statement.
InstructionNode*  parse_switch_stmt()
{
    Token t,t2,varName;
    t = expect(SWITCH);
    varName = expect(ID);
    t = expect(LBRACE);

    //new stuff
    InstructionNode* inst = new InstructionNode;
    InstructionNode* caseList = new InstructionNode;
    InstructionNode* noop = new InstructionNode;
    noop->type=NOOP;
    noop->next = nullptr;

    inst = parse_case_list(noop,varName);
    if(inst->type == 0)
    {
        inst->type = NOOP;
        inst = noop;
    }

    t2 = lexer.peek(1);
    if(t2.token_type == DEFAULT)
    {

        if(inst->type == NOOP)
        {
            inst = parse_default_case(noop);
        }
        else
        {
            InstructionNode *tmp = new InstructionNode;
            tmp = inst;
            while (tmp->next != nullptr)
            {
                tmp = tmp->next;
            }
            tmp->next = parse_default_case(noop);
        }
    }
    else if (inst->type == 1000)
    {
       //inst->next = nullptr;
    }
    else
    {
        InstructionNode* tmp = new InstructionNode;
        tmp = inst;
        while(tmp->next != nullptr)
        {
            tmp = tmp->next;
        }
        tmp->next = noop;
    }



    t = expect(RBRACE);

    return inst;
}


//This function parses all the cases in a switch statement.
InstructionNode* parse_case_list(InstructionNode* noop, Token var)
{
    Token t;
    InstructionNode* inst = new InstructionNode;
    InstructionNode* caseList = new InstructionNode;

    inst = parse_case(noop,var);
    t = lexer.peek(1);
    if(t.token_type == CASE)
    {
      caseList = parse_case_list(noop,var);
      inst->next = caseList;
    }
    else if(t.token_type == RBRACE || t.token_type == DEFAULT)
    {
        return inst;
    }
    else
    {
        syntax_error();
    }
    return inst;
}

//This function parses a case for a switch statement and the body of the case.
InstructionNode* parse_case(InstructionNode* noop, Token var)
{
    Token t,peekr;
    int addr;
    InstructionNode* inst = new InstructionNode;


    peekr = lexer.peek(1);
    if(peekr.token_type == CASE)
    {

        inst->type = CJMP;

        addr = findAddress(var.lexeme);
        inst->cjmp_inst.operand1_index = addr; //first operand
        inst->cjmp_inst.condition_op = CONDITION_NOTEQUAL;
        inst->next = nullptr;




        t = expect(CASE);
        t = expect(NUM);
        location(t.lexeme);
        addr = findAddress(t.lexeme);
        if (addr != -1)
        {
            mem[addr] = stoi(t.lexeme);
        }
        inst->cjmp_inst.operand2_index = addr; //second operand

        t = expect(COLON);

        inst->cjmp_inst.target = parse_body();

        InstructionNode *jmp = new InstructionNode;
        jmp->type = JMP;
        jmp->jmp_inst.target = noop;
        jmp->next = nullptr;

        InstructionNode *temp = new InstructionNode;
        temp = inst->cjmp_inst.target;
        while (temp->next != nullptr)
        {
            temp = temp->next;
        }
        temp->next = jmp;

        if (inst->cjmp_inst.target->type == 0)
        {
            inst->cjmp_inst.target = noop;
        }
    }
    else
    {

    }
    return inst;
}

//This function parses the default case of a switch statement and the body of the default case.
InstructionNode*  parse_default_case(InstructionNode* noop)
{
    Token t;
    t = expect(DEFAULT);
    t = expect(COLON);

    InstructionNode* inst = new InstructionNode;

    inst = parse_body();

    InstructionNode* temp = new InstructionNode;
    temp = inst;
    while(temp->next != nullptr)
    {
        temp = temp->next;
    }
    temp->next = noop;

    if(inst->type == 0)
    {
        inst = noop;
    }

    return inst;
}

//This function parses a for-loop and the body of the for-loop.
InstructionNode* parse_for_stmt()
{
    Token t;
    InstructionNode* ass1 = new InstructionNode;
    InstructionNode* ass2 = new InstructionNode;
    InstructionNode* inst = new InstructionNode;


    t = expect(FOR);
    t = expect(LPAREN);
    ass1 = parse_assign_stmt();


    inst = parse_condition();
    t = expect(SEMICOLON);
    ass2 = parse_assign_stmt();
    t = expect(RPAREN);
    inst->next = parse_body();


    InstructionNode *jmp = new InstructionNode;
    jmp->type = JMP;
    jmp->jmp_inst.target = inst;

    InstructionNode *temp = new InstructionNode;
    temp = inst;
    while(temp->next != nullptr)
    {
        temp = temp->next;
    }
    temp->next = ass2;
    ass2->next = jmp;

    InstructionNode *noop = new InstructionNode;
    noop->type = NOOP;
    noop->next = nullptr;

    jmp->next = noop;

    inst->cjmp_inst.target = noop;

    ass1->next = inst;
    return ass1;

}

//This function parses an input statement
void parse_inputs()
{
    parse_num_list();
}

//This function parses the inputs list at the end of a program.
void parse_num_list()
{
    Token t,t2;
    t = expect(NUM);


    inputs.push_back(stoi(t.lexeme));


    t2 = lexer.peek(1);
    if (t2.token_type == NUM)
    {
        parse_num_list();
    }
    else if (t2.token_type == END_OF_FILE)
    {
    }
    else
    {
        syntax_error();
    }
}
//William Owen, CSE 340, Professor Bazzi.
//ASU ID: 1207961816