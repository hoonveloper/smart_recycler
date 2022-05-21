import time
import cv2
import numpy as np
import socket
import sys

det_class = ["Can", "PET"]


def Detect(YOLO_net, output_layers, det):
    # 카메라 프레임
    cap = cv2.VideoCapture(0)
    _, frame = cap.read()
    h, w, c = frame.shape
    # YOLO 입력
    blob = cv2.dnn.blobFromImage(frame, 0.00392, (416, 416), (0, 0, 0),
                                 True, crop=False)
    YOLO_net.setInput(blob)
    outs = YOLO_net.forward(output_layers)

    class_ids = []
    confidences = []
    boxes = []
    labels = []

    for out in outs:
        for detection in out:
            scores = detection[5:]
            class_id = np.argmax(scores)
            confidence = scores[class_id]

            if confidence > 0.5:
                print(confidence)
                # Object detected
                center_x = int(detection[0] * w)
                center_y = int(detection[1] * h)
                dw = int(detection[2] * w)
                dh = int(detection[3] * h)
                # Rectangle coordinate
                x = int(center_x - dw / 2)
                y = int(center_y - dh / 2)
                boxes.append([x, y, dw, dh])
                confidences.append(float(confidence))
                class_ids.append(class_id)
                labels.append(str(classes[class_id]))

    print(labels)
    indexes = cv2.dnn.NMSBoxes(boxes, confidences, 0.45, 0.4)

    ret = False
    for i in range(len(boxes)):
        if i in indexes:
            x, y, w, h = boxes[i]
            label = str(classes[class_ids[i]])
            score = confidences[i]

            if label != det:
                continue
            # 경계상자와 클래스 정보 이미지에 입력
            cv2.rectangle(frame, (x, y), (x + w, y + h), (0, 0, 255), 5)
            cv2.putText(frame, label, (x, y - 20), cv2.FONT_ITALIC, 2,
                        (0, 255, 0), 5)
            frame = cv2.resize(frame, dsize=(640, 480),
                               interpolation=cv2.INTER_AREA)
            # cv2.imshow('map', frame)
            cv2.imwrite(time.strftime('%y-%m-%d_%H:%M:%S.png'), frame)
            ret = True
    cap.release()
    cv2.imwrite('test.png', frame)
    return ret


if __name__ == '__main__':
    ip = sys.argv[1]
    port = int(sys.argv[2])
    server_addr = (ip, port)
    s = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
    s.connect(server_addr)

    YOLO_net = cv2.dnn.readNet("custom_final.weights", "custom2.cfg")

    # YOLO NETWORK 재구성
    classes = []
    with open("custom2.names", "r") as f:
        classes = [line.strip() for line in f.readlines()]
    layer_names = YOLO_net.getLayerNames()
    output_layers = [layer_names[i - 1]
                     for i in YOLO_net.getUnconnectedOutLayers()]

    print("Connected to {:s}".format(repr(server_addr)))

    _class = -1
    while True:
        buff = s.recv(1)
        print(buff)
        # 3 오면 사진 찍고 보내기
        # 1-2 오면 클래스 설정
        if buff == b'3':
            print("찰칵")
            # s.send("1".encode())
            if _class == -1:
                continue
            det = Detect(YOLO_net, output_layers, det_class[_class])
            print(det)
            if det == True:
                s.send("1".encode())
            else:
                s.send("0".encode())
        elif buff == b'1':
            _class = 0
            print(det_class[_class] + " 설정 완료")
        elif buff == b'2':
            _class = 1
            print(det_class[_class] + " 설정 완료")
