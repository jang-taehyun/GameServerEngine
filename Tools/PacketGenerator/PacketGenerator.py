import argparse
import jinja2
import ProtoParser

def main():

    arg_parser = argparse.ArgumentParser(description = 'PacketGanerator')
    arg_parser.add_argument('--path', type=str, default='D:/GameServerProject/Server/Common/ProtoBuf/Protocol.proto', help='proto path')
    arg_parser.add_argument('--output', type=str, default='TestPacketHandler', help='ouput file')
    arg_parser.add_argument('--recv', type=str, default='C_', help='recv convention')
    arg_parser.add_argument('--send', type=str, default='S_', help='send convention')
    args = arg_parser.parse_args()

    parser = ProtoParser.ProtoParser(1000, args.recv, args.send)
    parser.parse_proto(args.path)

    # jinja2 환경 설정
    file_loader = jinja2.FileSystemLoader('Templates')
    env = jinja2.Environment(loader=file_loader)

    # 패킷에 맞게 코드 생성
    template = env.get_template('PacketHandler.h')
    output = template.render(parser=parser, output=args.output)

    # 생성한 코드를 파일로 write
    f = open(args.output+'.h', 'w+', encoding='utf-8')
    f.write(output)
    f.close()

    # print(output)
    return

if __name__ == '__main__':
    main()