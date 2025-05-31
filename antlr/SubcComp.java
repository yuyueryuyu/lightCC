import org.antlr.v4.runtime.*;
import org.antlr.v4.runtime.tree.*;
import java.io.IOException;

public class SubcComp {
    public static void main(String[] args) throws IOException {

        // 输入方式二：从标准输入读取
        CharStream charStream = CharStreams.fromStream(System.in);

        // 输入方式三：从文件读取
        //CharStream charStream = CharStreams.fromFileName("../test/00_bitset1.sy");

        // 创建 Lexer 实例
        SubcLexer lexer = new SubcLexer(charStream);
        
        // 获取所有词法单元
        Vocabulary vocab = lexer.getVocabulary();
        CommonTokenStream tokens = new CommonTokenStream(lexer);
        tokens.fill();

        // 遍历并打印词法单元
        for (Token token : tokens.getTokens()) {
            String tokenName = vocab.getSymbolicName(token.getType());
            System.out.printf(
                "[@%d,%d:%d='%s',<%s>,%d:%d]%n",
                token.getTokenIndex(),
                token.getStartIndex(),
                token.getStopIndex(),
                token.getText().replace("\n", "\\n"),
                tokenName,
                token.getLine(),
                token.getCharPositionInLine()
            );
        }
    }
}