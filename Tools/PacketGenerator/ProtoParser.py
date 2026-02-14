

class ProtoParser():
    def __init__(self, start_id, recv_prefix, send_prefix):
        self.recv_pkt = []              # 수신 패킷 목록(배열)
        self.send_pkt = []              # 송신 패킷 목록(배열)
        self.total_pkt = []             # 모든 패킷 목록(배열)
        self.start_id = start_id
        self.id = start_id;
        self.recv_prefix = recv_prefix
        self.send_prefix = send_prefix
        
    def parse_proto(self, path):
         f = open(path, 'r')
         lines = f.readlines()

         for line in lines:
             # 패킷인지 판단
             if line.startswith('message') == False:
                 continue

             pkt_name = line.split()[1].upper()

             # 수신 패킷인지 송신 패킷인지 구분해서 목록에 넣기
             if pkt_name.startswith(self.recv_prefix):
                 self.recv_pkt.append(Packet(pkt_name, self.id))
             elif pkt_name.startswith(self.send_prefix):
                 self.send_pkt.append(Packet(pkt_name, self.id))
             else:
                 continue

             # 전체 패킷 목록에 넣기(전체 패킷 목록은 enum을 만들기 위해 사용)
             self.total_pkt.append(Packet(pkt_name, self.id))
             self.id += 1
         
         f.close()

class Packet:
    def __init__(self, name, id):
        self.name = name
        self.id = id