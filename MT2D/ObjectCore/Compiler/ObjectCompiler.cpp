#include <MT2D/ObjectCore/Compiler/AbstractObject.h>
#include <MT2D/ObjectCore/Compiler/RawObject.h>
#include <stdlib.h>
#include <string.h>

struct DataBlock
{
    char **data;
    int Count;
    int RawObjectStartLine;
};

enum LineType
{
    LineTypeComment,
    LineTypeVariable,
    LineTypeConstructor,
    LineTypeState,
    LineTypeUnknown
};

char *GetVariableName(char *line, int *startAt);


AbstractObject *MT2D_ObjectCompiler_Deserialize(RawObject *object)
{
    AbstractObject *deserializedObject = (AbstractObject*)malloc(sizeof(AbstractObject));
    MT2D_VAR *parsedVariable = 0;
    int lineToParse = 0;
    int endBracket = 0;
    LineType lineType;
    while(lineToParse < object->Count)
    {
        lineType = GetLineType(object->Lines[lineToParse], object->Name);
        switch (lineType)
        {
        case LineTypeVariable:
            parsedVariable = TryParseVariable(object->Lines[lineToParse]);
            if (parsedVariable == 0)
            {
                //Log Error
            }
            else
            {
                //Add variable
                lineToParse++;
            }
            break;
        case LineTypeConstructor:
            endBracket = GetEndBracketLine(object->Lines, lineToParse + 2, object->Count);
            if (endBracket == -1)
            {
                // Log error not found end
                exit(-1);
            }
            else
            {
                // Parse the constructor
                ParseObjectConstructor(object, deserializedObject, lineToParse + 1, endBracket);
                lineToParse = endBracket + 1;
            }
            break;
        case LineTypeState:
            endBracket = GetEndBracketLine(object->Lines, lineToParse + 2, object->Count);
            if (endBracket == -1)
            {
                // Log error not found end
                exit(-1);
            }
            else
            {
                // Parse the constructor
                lineToParse = endBracket + 1;
            }
            break;
        default:
            break;
        }

    }


}
void MT2D_ObjectCompiler_Register(AbstractObject *object)
{
}

void MT2D_ObjectCompiler_Remove(char *objectName)
{

}

int GetEndBracketLine(char **data, int startAt, int limit)
{
    int bracketOpen = 0;
    for(int i = startAt; i < limit; i++)
    {
        if(data[i][0] == '{')
        {
            bracketOpen++;
        }
        else if(data[i][0] == '}')
        {
            bracketOpen--;
            if(bracketOpen == -1)
            {
                return i;
            }
        }
    }
    return -1;
}

MT2D_VAR *TryParseVariable(char *line)
{
    MT2D_VAR* variable = 0;
    char *name;
    int index;
    if (line[0] == 'i' && line[1] == 'n' && line[2] == 't')
    {
        index = 4;
        name = GetVariableName(line, &index);
        variable = MT2D_Object_Create_Var_Int(name, atoi(&line[index]));
    }
    else if(line[0] == 's' && line[1] == 't' && line[2] == 'r')
    {
        index = 8;
        name = GetVariableName(line, &index);
    }
    return variable;
}

void TrySetInternalVariable(char *line, AbstractObject *deserializedObject)
{
    //TODO: flaky
    if (strncmp(line, "this->", 6) == 0)
    {
        if (strncmp(&line[7], "Size.X", 6 ) == 0)
        {
            *(int*)deserializedObject->DefaultUserVars[Object_SizeX_Var_Index].Data = atoi(&line[13]);
        }
        else if (strncmp(&line[7], "Size.Y", 6 ) == 0)
        {
            *(int*)deserializedObject->DefaultUserVars[Object_SizeY_Var_Index].Data = atoi(&line[13]);
        }
        else if (strncmp(&line[7], "Enabled", 7 ) == 0)
        {
            //need to parse it
            deserializedObject->DefaulIsEnabled = true;
        }
        else if (strncmp(&line[7], "Visible", 7 ) == 0)
        {
            //need to parse it
            deserializedObject->DefaultRenderOnly = true;
        }
        else
        {
            //drop error.
        }

    }
}

bool ParseObjectConstructor(RawObject *object, AbstractObject *deserializedObject, int startAt, int endAt)
{
    int i = startAt;
    LineType type;
    while( i < endAt)
    {
        type = GetLineType(object->Lines[i], object->Name);
        if(type == LineTypeVariable)
        {
            TrySetInternalVariable(object->Lines[i], deserializedObject);
        }
        else if (type != LineTypeComment)
        {
            //Log error unknown data at line i.
        }
        i++;
    }
}

char *GetVariableName(char *line, int *startAt)
{
    char *name;
    int end;
    int offset = startAt[0] + 1;
    while(line[offset] != '\0' && line[offset] != ' ' && line[offset] != 11/*TAB*/)
    {
        offset++;
    }
    name = (char*)malloc((startAt[0] - offset + 1) * sizeof(char));
    strcpy(name, &line[startAt[0]]);
    startAt[0] = offset;
    return name;
}

LineType GetLineType(char *line, char *className)
{
    //This function will not validate the inner data inside of Constructor nor State
    int len = strlen(line);
    int classLen;
    if (len < 3)
    {
        return LineTypeComment;
    }
    else if (line[0] == '/' && line[1] == '/')
    {
        return LineTypeComment;
    }
    else if (line[len - 2] == '(' && line[len - 1] == ')')
    {
        if (len >= 7 && strncmp(line, "state", 5) == 0)
        {
            return LineTypeState;
        }
        classLen = strlen(className);
        if (classLen + 2 == len && strncmp(line, className, classLen) == 0)
        {
            return LineTypeConstructor;
        }
        return LineTypeUnknown;
    }
    return LineTypeVariable;
}

char *FilterString(char *line)
{
    char *string = 0;
    int len = strlen(line);
    int i = 0;
    int start;
    int end =0;
    while(line[i] != '\"' && i < len){
        i++;
    }
    start = i;
    i++;
    while(line[i] != '\"' && i < len){
        i++;
    }
    end = i;
    if (end < len){
        string = (char*)malloc((end - start) * sizeof(char));
        strncpy(string, &line[start], end - start);
    }
    return string;
}