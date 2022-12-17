a.out : consumer producer
consumer : consumer.c
		gcc	-o consumer consumer.c common.h -lm
producer : producer.c
		gcc	-o producer producer.c common.h -lm