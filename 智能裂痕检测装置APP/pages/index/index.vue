<template>
	<view class="wrap">
		<view class="dev-area">

			<view class="dev-cart">
				<view class="">
					<view class="dev-name">今日有裂痕齿轮数量</view>
					<image class="dev-logo" src="../../static/crack.png" mode=""></image>
				</view>
				<view class="dev-data">{{ crack }}个</view>
			</view>
			<view class="dev-cart">
				<view class="">
					<view class="dev-name">今日有裂痕的标号</view>
					<image class="dev-logo" src="../../static/tab.png" mode=""></image>
				</view>
				<view class="dev-data">{{ tab }} </view>
			</view>
			<view class="dev-cart">
				<view class="">
					<view class="dev-name">云台向下旋转</view>
					<image class="dev-logo" src="../../static/down.png" mode=""></image>
				</view>
				<switch :checked=" down " @change="Switch1" color="#2b9939"/>
			</view>
			<view class="dev-cart">
				<view class="">
					<view class="dev-name">云台向左旋转</view>
					<image class="dev-logo" src="../../static/left.png" mode=""></image>
				</view>
				<switch :checked="left" @change="Switch2" color="#2b9939"/>
			</view>
			<view class="dev-cart">
				<view class="">
					<view class="dev-name">云台向右旋转</view>
					<image class="dev-logo" src="../../static/right.png" mode=""></image>
				</view>
				<switch :checked=" right " @change="Switch3" color="#2b9939"/>
			</view>
			<view class="dev-cart">
				<view class="">
					<view class="dev-name">云台向上旋转</view>
					<image class="dev-logo" src="../../static/up.png" mode=""></image>
				</view>
				<switch :checked="up" @change="Switch4" color="#2b9939"/>
			</view>
			<view class="dev-cart">
				<view class="">
					<view class="dev-name">是否找到人</view>
					<image class="dev-logo" src="../../static/find.png" mode=""></image>
				</view>
				<view class="dev-data">{{people}} </view>
			</view>
			</view>

		</view>
	</view>

</template>

<script>
	const {
		createCommonToken
	} = require('@/key.js')
	
	const product_id = 'BOPTVcAZTj';
	const device_name = 'd2';
	const access_key = 'he0B8Y2B/cFhn0fr3fk5M9v9WQujY+Ox/zPJU1VR7W0=';
	
	export default {
		data() {
			return {
				crack: '',
				down: '',
				left: '',
				right: '',
				tab: '',
				up: '',
				people: '',
				token: '',
			}
		},
		onLoad() {
			const params = {
				access_key: 'he0B8Y2B/cFhn0fr3fk5M9v9WQujY+Ox/zPJU1VR7W0=',
				version: '2022-05-01',
				productid: 'BOPTVcAZTj', 
			}
			this.token = createCommonToken(params);
		},
		onShow() {
			this.fetchDevData();
			setInterval(()=>{
				this.fetchDevData();
			},3000)
		},
		methods: {
			fetchDevData() {
				uni.request({
					url: 'https://iot-api.heclouds.com/thingmodel/query-device-property',
					method: 'GET',
					data: {
						product_id: 'BOPTVcAZTj',
						device_name: 'd2'
					},
					header: {
						'authorization': this.token
					},
					success: (res) => {
						console.log(res.data);
						this.crack = res.data.data[0].value;
						this.down = res.data.data[1].value === 'false';
						this.left = res.data.data[2].value === 'false';
						this.right = res.data.data[4].value === 'false';
						this.tab = res.data.data[5].value;
						this.up = res.data.data[6].value === 'false';
						this.people = res.data.data[3].value;
					}
				});
			},
			Switch1(event) {
				console.log(event.detail.value);
				let value = event.detail.value;
				uni.request({
					url: 'https://iot-api.heclouds.com/thingmodel/set-device-property',
					method: 'POST',
					data: {
						product_id: product_id,
						device_name: device_name,
						params: {
							"down": value
						}
					},
					header: {
						'authorization': this.token
					},
					success: () => {
						console.log('down ' + (value ? 'ON' : 'OFF') + ' !');
					}
				});
			},
			Switch2(event) {
				console.log(event.detail.value);
				let value = event.detail.value;
				uni.request({
					url: 'https://iot-api.heclouds.com/thingmodel/set-device-property',
					method: 'POST',
					data: {
						product_id: product_id,
						device_name: device_name,
						params: {
							"left": value
						}
					},
					header: {
						'authorization': this.token
					},
					success: () => {
						console.log('left ' + (value ? 'ON' : 'OFF') + ' !');
					}
				});
			},
			Switch3(event) {
				console.log(event.detail.value);
				let value = event.detail.value;
				uni.request({
					url: 'https://iot-api.heclouds.com/thingmodel/set-device-property',
					method: 'POST',
					data: {
						product_id: product_id,
						device_name: device_name,
						params: {
							"right": value
						}
					},
					header: {
						'authorization': this.token
					},
					success: () => {
						console.log('right ' + (value ? 'ON' : 'OFF') + ' !');
					}
				});
			},
			Switch4(event) {
				console.log(event.detail.value);
				let value = event.detail.value;
				uni.request({
					url: 'https://iot-api.heclouds.com/thingmodel/set-device-property',
					method: 'POST',
					data: {
						product_id: product_id,
						device_name: device_name,
						params: {
							"up": value
						}
					},
					header: {
						'authorization': this.token
					},
					success: () => {
						console.log('up ' + (value ? 'ON' : 'OFF') + ' !');
					}
				});
			}
		}
	}
</script>

<style>
	.wrap {
		padding: 30rpx;
	}

	.dev-area {
		display: flex;
		justify-content: space-between;
		flex-wrap: wrap;
	}

	.dev-cart {
		height: 150rpx;
		width: 320rpx;
		border-radius: 30rpx;
		margin-top: 30rpx;
		display: flex;
		justify-content: space-around;
		align-items: center;
		box-shadow: 0 0 15rpx #ccc;
	}

	.dev-name {
		font-size: 20rpx;
		text-align: center;
		color: #6d6d6d;
	}

	.dev-logo {
		width: 70rpx;
		height: 70rpx;
		margin-top: 10rpx;
	}

	.dev-data {
		font-size: 50rpx;
		color: #6d6d6d;
	}

</style>